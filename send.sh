#!/bin/bash

echo "*********** Send a folder to your Raspberry Pi ***********"

# Configuration
PI_IP="192.168.1.127"
PI_USER="pi"
DEST_PATH="/home/pi/Desktop/device_drivers"
DIST="$PI_USER@$PI_IP:$DEST_PATH"
KNOWN_HOSTS="$HOME/.ssh/known_hosts"

# Check Pi is reachable
echo "Checking if $PI_IP is reachable..."
ping -c 1 "$PI_IP" > /dev/null 2>&1
if [ $? -ne 0 ]; then
    echo "❌ Raspberry Pi ($PI_IP) is unreachable. Check the network."
    exit 1
fi

# Clean known_hosts if needed
echo "Making sure there's no old SSH fingerprint conflict..."
ssh-keygen -f "$KNOWN_HOSTS" -R "$PI_IP" > /dev/null 2>&1

# Get new fingerprint silently
ssh-keyscan -H "$PI_IP" >> "$KNOWN_HOSTS" 2>/dev/null

# List folders and ask user
echo "Available folders in current directory:"
ls -d */ 2>/dev/null | sed 's#/##'

read -rp "Enter the folder name you want to send: " folder_name

# Check and send
if [ -d "$folder_name" ]; then
    echo "Sending folder '$folder_name' to $DIST"
    scp -r "$folder_name" "$DIST"
    if [ $? -eq 0 ]; then
        echo "✅ Folder sent successfully."
    else
        echo "❌ Failed to send folder. Check credentials or path."
    fi
else
    echo "❌ The specified folder does not exist: $folder_name"
    exit 1
fi
