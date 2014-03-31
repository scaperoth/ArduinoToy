setlocal
set PATH=%PATH%;C:\Program Files (x86)\Arduino\
javac -cp "C:\Program Files (x86)\Arduino\lib\RXTXcomm.jar;." SerialTest.java
"C:\Program Files (x86)\Java\jre7\bin\java" -cp "C:\Program Files (x86)\Arduino\lib\RXTXcomm.jar;." SerialTest