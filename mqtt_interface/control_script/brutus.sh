#!/bin/bash

# --------------------------
# MQTT CONFIGURATION
# --------------------------
MQTT_HOST="test.mosquitto.org"
MQTT_PORT="1883"

TOPIC_CMD_POSE="brutus/cmd/pose"
TOPIC_CMD_MODE="brutus/cmd/mode"
TOPIC_CMD_VEL="brutus/cmd/velocity"
TOPIC_STATE_MODE="brutus/state/mode"

TOPIC_PUB_HEARTBEAT="brutus/data/heartbeat"
TOPIC_PUB_POSE="brutus/data/pose"
TOPIC_PUB_FRONT="brutus/data/distance/front"
TOPIC_PUB_RIGHT="brutus/data/distance/right"
TOPIC_PUB_LEFT="brutus/data/distance/left"

# --------------------------
# MQTT FUNCTIONS
# --------------------------
publish() {
    mosquitto_pub -h "$MQTT_HOST" -p "$MQTT_PORT" -t "$1" -m "$2"
}

subscribe() {
    echo "Listening to Brutus publications..."
    mosquitto_sub -h "$MQTT_HOST" -p "$MQTT_PORT" \
        -t "$TOPIC_PUB_HEARTBEAT" \
        -t "$TOPIC_PUB_POSE" \
        -t "$TOPIC_PUB_FRONT" \
        -t "$TOPIC_PUB_RIGHT" \
        -t "$TOPIC_PUB_LEFT"
}

# --------------------------
# SEND FUNCTIONS
# --------------------------
send_vel() {
    read -p "Enter linear velocity X: " vx
    read -p "Enter angular velocity Z: " wz
    JSON="{\"vx\":$vx,\"wz\":$wz}"
    publish "$TOPIC_CMD_VEL" "$JSON"
    echo "Velocity sent: $JSON"
}

send_mode() {
    read -p "Mode (e.g. AUTO / MANUAL / STOP): " mode
    publish "$TOPIC_CMD_MODE" "\"$mode\""
    echo "Mode sent: $mode"
}

send_state_mode() {
    read -p "Mode state: " state
    publish "$TOPIC_STATE_MODE" "\"$state\""
    echo "State sent: $state"
}

# --------------------------
# DYNAMIC POSE EDITOR
# --------------------------
send_pose() {
    # Initialize joints at 90
    declare -A joints=(
        [FR_SH]=90 [FR_EL]=90
        [FL_SH]=90 [FL_EL]=90
        [BR_SH]=90 [BR_EL]=90
        [BL_SH]=90 [BL_EL]=90
    )

    keys=(FR_SH FR_EL FL_SH FL_EL BR_SH BR_EL BL_SH BL_EL)
    pos=0

    draw() {
        clear
        echo "=============================================="
        echo "         BRUTUS DYNAMIC POSE EDITOR           "
        echo "=============================================="
        echo "Use arrows ↑↓ to move, ←→ to change value"
        echo "Press q to return to main menu"
        echo
        for i in "${!keys[@]}"; do
            key=${keys[$i]}
            val=${joints[$key]}
            if [ $i -eq $pos ]; then
                echo -e " → $key : $val"
            else
                echo "   $key : $val"
            fi
        done
        echo
    }

    send_pose_mqtt() {
        JSON="{\"fr\":{\"ang_shoulder\":${joints[FR_SH]},\"ang_elbow\":${joints[FR_EL]}},\
\"fl\":{\"ang_shoulder\":${joints[FL_SH]},\"ang_elbow\":${joints[FL_EL]}},\
\"br\":{\"ang_shoulder\":${joints[BR_SH]},\"ang_elbow\":${joints[BR_EL]}},\
\"bl\":{\"ang_shoulder\":${joints[BL_SH]},\"ang_elbow\":${joints[BL_EL]}}}"
        publish "$TOPIC_CMD_POSE" "$JSON"
    }

    draw
    while true; do
        read -rsn1 key1
        if [[ $key1 == $'\x1b' ]]; then
            read -rsn2 -t 0.1 key2
            case "$key2" in
                '[A') ((pos--)); if [ $pos -lt 0 ]; then pos=$((${#keys[@]}-1)); fi ;; # Up
                '[B') ((pos++)); if [ $pos -ge ${#keys[@]} ]; then pos=0; fi ;;          # Down
                '[C') ((joints[${keys[$pos]}]++)); send_pose_mqtt ;;  # Right increases
                '[D') ((joints[${keys[$pos]}]--)); send_pose_mqtt ;;  # Left decreases
            esac
        elif [[ $key1 == "q" ]]; then
            clear
            return
        fi
        draw
    done
}

# --------------------------
# MAIN MENU
# --------------------------
while true; do
    clear
    echo "=============================================="
    echo "          BRUTUS CONTROL INTERFACE        "
    echo "=============================================="
    echo "Select an option:"
    echo
    echo " 1) Send full pose"
    echo " 2) Send velocity {vx, wz}"
    echo " 3) Send mode"
    echo " 4) Send mode state"
    echo "----------------------------------------------"
    echo " 5) Listen to robot publications"
    echo "----------------------------------------------"
    echo " 0) Exit"
    echo

    read -p "Option: " opt

    case "$opt" in
        1) send_pose ;;
        2) send_vel ;;
        3) send_mode ;;
        4) send_state_mode ;;
        5) subscribe ;;
        0) clear; exit 0 ;;
        *) echo "Invalid option" ;;
    esac

    read -p "Press ENTER to continue..."
done
msg