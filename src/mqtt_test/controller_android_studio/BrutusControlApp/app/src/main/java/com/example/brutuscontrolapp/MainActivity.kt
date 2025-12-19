package com.example.brutuscontrolapp

import android.graphics.Color
import android.os.Bundle
import android.os.Handler
import android.os.Looper
import android.widget.*
import androidx.appcompat.app.AppCompatActivity
import androidx.core.view.setPadding
import com.hivemq.client.mqtt.MqttClient
import com.hivemq.client.mqtt.mqtt3.Mqtt3AsyncClient
import com.hivemq.client.mqtt.mqtt3.message.publish.Mqtt3Publish
import java.nio.charset.StandardCharsets

class MainActivity : AppCompatActivity() {

    private lateinit var mqttClient: Mqtt3AsyncClient
    private val clientId = "android_brutus"

    private val TOPIC_CMD_POSE = "brutus/cmd/pose"
    private val TOPIC_CMD_MODE = "brutus/cmd/mode"
    private val TOPIC_CMD_VEL = "brutus/cmd/velocity"
    private val TOPIC_PUB_HEARTBEAT = "brutus/data/heartbeat"
    private val TOPIC_PUB_POSE = "brutus/data/pose"
    private val TOPIC_DIST_FRONT = "brutus/data/distance/front"
    private val TOPIC_DIST_RIGHT = "brutus/data/distance/right"
    private val TOPIC_DIST_LEFT = "brutus/data/distance/left"

    private lateinit var tvConnection: TextView
    private lateinit var tvReceived: TextView
    private lateinit var layoutPose: LinearLayout

    private val MODE_STANDING = "0"
    private val MODE_EX1 = "1"
    private val MODE_EX2 = "2"
    private val MODE_LEGS = "3"
    private val MODE_SPEED = "4"

    private val joints = mutableMapOf(
        "FR_SH" to 0f, "FR_EL" to 0f,
        "FL_SH" to 0f, "FL_EL" to 0f,
        "BR_SH" to 0f, "BR_EL" to 0f,
        "BL_SH" to 0f, "BL_EL" to 0f
    )
                private val seekMap = mutableMapOf<String, SeekBar>()
    private val handler = Handler(Looper.getMainLooper())

    // Guardar último mensaje por topic
    private val lastMessages = mutableMapOf<String, String>()

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        tvConnection = findViewById(R.id.tvConnection)
        tvReceived = findViewById(R.id.tvReceived)
        layoutPose = findViewById(R.id.layoutPose)

        setupMQTT()
        setupUI()
    }

    private fun setupMQTT() {
        mqttClient = MqttClient.builder()
            .useMqttVersion3()
            .identifier(clientId)
            .serverHost("test.mosquitto.org")
            .serverPort(1883)
            .buildAsync()

        mqttClient.connect()
            .whenComplete { _, ex ->
                handler.post {
                    if (ex == null) {
                        tvConnection.text = "MQTT: Connected"
                        tvConnection.setTextColor(Color.GREEN)

                        subscribeTopic(TOPIC_PUB_HEARTBEAT)
                        subscribeTopic(TOPIC_PUB_POSE)
                        subscribeTopic(TOPIC_DIST_FRONT)
                        subscribeTopic(TOPIC_DIST_RIGHT)
                        subscribeTopic(TOPIC_DIST_LEFT)
                    } else {
                        tvConnection.text = "MQTT: Connection failed"
                        tvConnection.setTextColor(Color.RED)
                        ex.printStackTrace()
                    }
                }
            }
    }

    private fun subscribeTopic(topic: String) {
        mqttClient.subscribeWith()
            .topicFilter(topic)
            .callback { publish: Mqtt3Publish ->
                val payload = String(publish.payloadAsBytes, StandardCharsets.UTF_8)

                handler.post {
                    // Guardar el último mensaje del topic
                    lastMessages[topic] = payload

                    // Reconstruir el TextView con los últimos mensajes
                    val displayText = lastMessages.entries.joinToString("\n\n") { (t, m) ->
                        "$t\n$m"
                    }.trim()
                    tvReceived.text = displayText
                }
            }
            .send()
    }

    private fun publishMessage(topic: String, message: String) {
        mqttClient.publishWith()
            .topic(topic)
            .payload(message.toByteArray())
            .send()
    }

    private fun setupUI() {
        // Botones de modos
        findViewById<Button>(R.id.btnModeStanding).setOnClickListener { publishMessage(TOPIC_CMD_MODE, MODE_STANDING) }
        findViewById<Button>(R.id.btnModeEx1).setOnClickListener { publishMessage(TOPIC_CMD_MODE, MODE_EX1) }
        findViewById<Button>(R.id.btnModeEx2).setOnClickListener { publishMessage(TOPIC_CMD_MODE, MODE_EX2) }
        findViewById<Button>(R.id.btnModeLegs).setOnClickListener { publishMessage(TOPIC_CMD_MODE, MODE_LEGS) }
        findViewById<Button>(R.id.btnModeSpeed).setOnClickListener { publishMessage(TOPIC_CMD_MODE, MODE_SPEED) }

        // Velocidades con valor al lado (-1.0 a 1.0)
        val seekVelX = findViewById<SeekBar>(R.id.seekVelX)
        val seekVelZ = findViewById<SeekBar>(R.id.seekVelZ)
        val tvVelX = findViewById<TextView>(R.id.tvVelXValue)
        val tvVelZ = findViewById<TextView>(R.id.tvVelZValue)

        seekVelX.max = 100
        seekVelX.progress = 50
        seekVelZ.max = 100
        seekVelZ.progress = 50

        fun progressToFloat(progress: Int): Float {
            return (progress - 50) / 50f
        }

        tvVelX.text = "0.0"
        tvVelZ.text = "0.0"

        seekVelX.setOnSeekBarChangeListener(object : SeekBar.OnSeekBarChangeListener {
            override fun onProgressChanged(sb: SeekBar?, progress: Int, fromUser: Boolean) {
                tvVelX.text = String.format("%.2f", progressToFloat(progress))
            }
            override fun onStartTrackingTouch(sb: SeekBar?) {}
            override fun onStopTrackingTouch(sb: SeekBar?) {}
        })

        seekVelZ.setOnSeekBarChangeListener(object : SeekBar.OnSeekBarChangeListener {
            override fun onProgressChanged(sb: SeekBar?, progress: Int, fromUser: Boolean) {
                tvVelZ.text = String.format("%.2f", progressToFloat(progress))
            }
            override fun onStartTrackingTouch(sb: SeekBar?) {}
            override fun onStopTrackingTouch(sb: SeekBar?) {}
        })

        findViewById<Button>(R.id.btnSendVel).setOnClickListener {
            val vx = progressToFloat(seekVelX.progress)
            val wz = progressToFloat(seekVelZ.progress)
            val json = "{\"vx\":$vx,\"wz\":$wz}"
            publishMessage(TOPIC_CMD_VEL, json)
        }

        // Joints - sliders de -1 a 1, centrados en 0
        for ((joint, _) in joints) {
            val container = LinearLayout(this)
            container.orientation = LinearLayout.HORIZONTAL
            container.setPadding(8)

            val tv = TextView(this)
            tv.text = "$joint: 0.0"
            tv.setTextColor(Color.BLUE)
            tv.textSize = 16f
            tv.layoutParams = LinearLayout.LayoutParams(0, LinearLayout.LayoutParams.WRAP_CONTENT, 1f)

            val seek = SeekBar(this)
            seek.max = 100
            seek.progress = 50  // centrado en 0
            seek.layoutParams = LinearLayout.LayoutParams(0, LinearLayout.LayoutParams.WRAP_CONTENT, 2f)

            fun progressToFloat(progress: Int): Float = (progress - 50) / 50f

            seek.setOnSeekBarChangeListener(object : SeekBar.OnSeekBarChangeListener {
                override fun onProgressChanged(sb: SeekBar?, progress: Int, fromUser: Boolean) {
                    val floatValue = progressToFloat(progress)
                    tv.text = "$joint: $floatValue"
                    joints[joint] = floatValue
                    tv.setTextColor(Color.parseColor("#1976D2"))
                    handler.removeCallbacksAndMessages(null)
                    handler.postDelayed({ tv.setTextColor(Color.BLUE) }, 500)
                    publishPose()
                }
                override fun onStartTrackingTouch(sb: SeekBar?) {}
                override fun onStopTrackingTouch(sb: SeekBar?) {}
            })

            container.addView(tv)
            container.addView(seek)
            layoutPose.addView(container)
            seekMap[joint] = seek
        }

        findViewById<Button>(R.id.btnPublishPose).setOnClickListener { publishPose() }
    }

    private fun publishPose() {
        val json = """
            {
                "fr": {"0": ${joints["FR_SH"]}, "1": ${joints["FR_EL"]}},
                "fl": {"0": ${joints["FL_SH"]}, "1": ${joints["FL_EL"]}},
                "br": {"0": ${joints["BR_SH"]}, "1": ${joints["BR_EL"]}},
                "bl": {"0": ${joints["BL_SH"]}, "1": ${joints["BL_EL"]}}
            }
        """.trimIndent()
        publishMessage(TOPIC_CMD_POSE, json)
    }
}
