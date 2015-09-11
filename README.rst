Configuring TWS
===============

1. Open The "Edit" menu
2. Click "Global Configurations" and a window will open
3. Open "API" and click "Settings"
4. Activate "Enable ActiveX and Socket Clients"


Install nkny
============

1. Download nkny_installer.exe and run
2. For install location, click the Browse button and create a folder called "nkny" on your desktop
3. finish the install with default by simply pressing "Next" or "Ok"
4. Navigate to the "nkny" folder and open it
5. Double click the "nkny.exe" application to open it



Windows 7 Registry Change
=========================

Open Windows Registry Editor
----------------------------

http://pcsupport.about.com/od/registry/ht/open-registry-editor.htm
http://stackoverflow.com/questions/18985816/change-default-socket-buffer-size-under-windows

1. Left click the "Start" button on windows desktop
2. In the search box, type "Run"
3. Under "Programs", left click the "Run" program
4. Type "%systemroot%\SysWOW64\regedit.exe", and click "Ok"
5. Open the "HKEY_LOCAL_MACHINE" folder
6. Open the "SYSTEM" folder
7. Open the "CurrentControlSet" folder
8. Open the "services" folder
9. Open the "AFD" folder
10. Open the "Parameters" folder
11. In the main window, right-click and press the "New" entry and then "DWORD (32-bit) Value"
12. Enter "DefaultReceiveWindow" for the Name
13. Right-click the new entry and click the "Decimal" radio button
14. Now, for the value enter "32768"
15. Repeat steps 11 through 14, but this time enter "DefaultSendWindow"
16. Close all programs and reboot Windows 7
