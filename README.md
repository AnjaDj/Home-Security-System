# Home Security Real-Time System🏠🔒 ➡️ DEVELOPMENT
We are developing a Home Security System where the entry process is carefully monitored for safety. When someone opens the door and steps into the room, an IR Distance Sensor detects their movement and sends a signal to a Raspberry Pi 3B. This triggers the main thread, which starts a timer thread set to 30 seconds.
During this time window, the individual has the opportunity to input the correct password. If the password is entered correctly within 30 seconds, the security protocol is deactivated. ✅ <br>
However, if the time expires without a valid password, the system activates ALARM RED-LED & BUZZER, signaling a breach 🚨 <br> <br> <br> 
For more detailed informations about each used modul, visit Wiki 📜 page. <br>
For more information about used hardware visit 🏃‍♀️ : <br>
BCM2837 Datasheet : https://www.alldatasheet.com/datasheet-pdf/pdf/1572343/BOARDCOM/BCM2837.html <br>
ADC 12 Click Mikroe : https://www.mikroe.com/adc-12-click?srsltid=AfmBOopHdVjXTjusoUrr0BsAWu4COvaQ-Fox8xVBRag2l970xT6AKSI3 <br>
Raspberry Pi Camera Rev 1.3 : <ul>
<li>https://www.raspberrypi.com/documentation/accessories/camera.html (hardware)    </li>                                                        
<li>https://www.raspberrypi.com/documentation/computers/camera_software.html (software) </li>
</ul>
Guide for setting up camera : https://community.element14.com/products/devtools/technicallibrary/m/files/16122 <br>
