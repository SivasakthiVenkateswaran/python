import network
import socket
import esp
import machine
import urequests
import time
from machine import I2C, Pin
from lcd_api import LcdApi
from i2c_lcd import I2cLcd

# Set up the LCD object with the pin configuration
I2C_ADDR = 0x27
I2C_NUM_ROWS = 2
I2C_NUM_COLS = 16

i2c = I2C(scl=Pin(5), sda=Pin(4), freq=400000)
lcd = I2cLcd(i2c, I2C_ADDR, I2C_NUM_ROWS, I2C_NUM_COLS)

ssid = "a"
password = "1234567890"
apSSID = "NodeMCU_AP"
apPassword = "AP_Password"
flaskServerAddress = ""

def read_string_from_serial():
    print("Enter Flask server address (e.g., http://192.168.1.100:5000):")
    input_string = input()
    return input_string

def handle_request(client, addr):
    request = client.recv(1024)
    request_str = str(request)
    client_ip = addr[0]
    print("Client IP:", client_ip)
    
    # Forward request to Flask server
    method = request_str.split(' ')[0]
    uri = request_str.split(' ')[1]
    headers = {'Content-Type': 'application/x-www-form-urlencoded', 'Client-MAC': 'UNKNOWN'}
    
    response = ""
    if method == 'GET':
        print("Requested URL:", uri)
        lcd.clear()
        lcd.putstr("--" + uri)
        try:
            response = urequests.get(flaskServerAddress + uri, headers=headers)
            client.send(b'HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n' + response.content)
        except Exception as e:
            client.send(b'HTTP/1.1 500 Internal Server Error\r\n\r\nFailed to reach Flask server')
        finally:
            response.close()
    elif method == 'POST':
        try:
            content_length = int(request_str.split('Content-Length: ')[1].split('\\r\\n')[0])
            post_data = request_str.split('\\r\\n\\r\\n')[1][:content_length]
            response = urequests.post(flaskServerAddress + uri, data=post_data, headers=headers)
            client.send(b'HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n' + response.content)
        except Exception as e:
            client.send(b'HTTP/1.1 500 Internal Server Error\r\n\r\nFailed to reach Flask server')
        finally:
            response.close()
    else:
        client.send(b'HTTP/1.1 405 Method Not Allowed\r\n\r\n')
    
    client.close()

def main():
    global flaskServerAddress
    flaskServerAddress = read_string_from_serial()

    sta_if = network.WLAN(network.STA_IF)
    sta_if.active(True)
    sta_if.connect(ssid, password)

    while not sta_if.isconnected():
        time.sleep(1)
        print("Connecting to WiFi...")

    print("Connected to WiFi")
    print(sta_if.ifconfig())

    ap = network.WLAN(network.AP_IF)
    ap.active(True)
    ap.config(essid=apSSID, password=apPassword)
    
    addr = socket.getaddrinfo('0.0.0.0', 80)[0][-1]
    s = socket.socket()
    s.bind(addr)
    s.listen(1)

    print("Access Point Started")
    print('listening on', addr)
    
    while True:
        client, addr = s.accept()
        handle_request(client, addr)

main()
