# Home-Security-System 🏠🔒 Real-Time Programming


We are developing a Home Security System where the entry process is carefully monitored for safety. When someone opens the door and steps into the room, an IR Distance Sensor detects their movement and sends a signal to a Raspberry Pi 3B. This triggers the main thread, which starts a timer thread set to 30 seconds.

During this time window, the individual has the opportunity to input the correct password. If the password is entered correctly within 30 seconds, the security protocol is deactivated. ✅ <br />
However, if the time expires without a valid password, the system activates ALARM RED-LED & BUZZER, signaling a breach 🚨
