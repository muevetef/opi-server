import pyzbar.pyzbar as pyzbar
import numpy as np
import cv2, time
import socketio
import base64
import sys
import zlib 

cap = cv2.VideoCapture(0) 
cap.set(3,320)
cap.set(4,240)
time.sleep(2)

def decode(im) : 
    # Find barcodes and QR codes
    decodedObjects = pyzbar.decode(im)
    # Print results
    for obj in decodedObjects:
        print('Type : ', obj.type)
        print('Data : ', obj.data,'\n')     
    return decodedObjects
font = cv2.FONT_HERSHEY_SIMPLEX

sio = socketio.Client()

#sio.connect('http://172.104.205.29:3000')
#sio.connect('http://192.168.0.170:3000')
sio.connect('http://192.168.1.131:3000')
print('my sid is', sio.sid)
sio.emit('storeClientInfo',{'name': 'cam0'})
isConnected=True
@sio.event
def connect():
    print("I'm connected!")
    sio.emit('storeClientInfo',{'name': 'cam0'})
    isConnected=True

@sio.event
def connect_error(data):
    print("The connection failed!")
    isConnected=False

@sio.event
def disconnect():
    print("I'm disconnected!")
    isConnected=False

@sio.on('stream_')
def on_message(data):
    print('I received a message!')
    print(data)

sio.wait()
frame_rate = 10
prev = 0
while(cap.isOpened() and isConnected):
        success, frame = cap.read()
        im = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
        decodedObjects = decode(im)

        for decodedObject in decodedObjects:
            points = decodedObject.polygon
            # If the points do not form a quad, find convex hull
            if len(points) > 4:
                hull = cv2.convexHull(np.array([point for point in points], dtype=np.float32))
                hull = list(map(tuple, np.squeeze(hull)))
            else:
                hull = points

            # Number of points in the convex hull
            n = len(hull)     
            # Draw the convext hull
            for j in range(0,n):
                cv2.line(frame, hull[j], hull[ (j+1) % n], (255,0,0), 3)

            x = decodedObject.rect.left
            y = decodedObject.rect.top

            print(x, y)

            print('Type : ', decodedObject.type)
            print('Data : ', decodedObject.data,'\n')

            barCode = str(decodedObject.data)
            cv2.putText(im, barCode, (x, y), font, 1, (0,255,255), 2, cv2.LINE_AA)   
            sio.emit('barcode', barCode)


        time_elapsed = time.time() - prev
        
        if time_elapsed > 1./frame_rate:
            prev = time.time()

            ret, buffer = cv2.imencode('.jpg', im)
            jpg_as_txt = base64.b64encode(buffer)
            cmp = zlib.compress(jpg_as_txt, -1)
            sio.emit('stream', cmp)
            #imageprint("no compress  ", sys.getsizeof(jpg_as_txt), " --  yes compress ", sys.getsizeof(cmp))
