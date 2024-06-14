#!/home/sudharsan/myenv/bin/python3
from flask import Flask, request,render_template, abort, redirect, url_for,session
import logging
import os
import db

log = logging.getLogger('werkzeug')
log.setLevel(logging.ERROR)
data={}
username=""
app = Flask(__name__, static_url_path='/static')
app.secret_key = b'_5#y2L"F4Q8z\n\xec]/'

static_folder = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'static')

'''
@app.before_request
def before_request_func():
    client_mac = request.headers.get('Client-MAC')
    print("Requester MAC:", client_mac)
    condition = db.check(client_mac)
    if not condition :  #and request.remote_addr!="172.217.28.11":
        print('undefined-user')
        abort(403)'''
        

@app.route('/',methods=['GET'])
@app.route('/generate204',methods=['GET'])
def captiveportal():
    return render_template('index.html')

@app.route('/login',methods=['POST'])
def login():
    global username
    client_mac = request.headers.get('Client-MAC')
    print("Requester MAC:", client_mac)
    data = request.form.to_dict()   
    print("Requested Data: " + str(data)) 
    if(db.login(data['name'],data['password'])):
        session['username']=data['name']
        username=data['name']
        return redirect(url_for('profile'))
    else:
        return "INVALID CREDENTIALS"
        
@app.route('/endpoint')
def test():
    print(session['username'])
    return 'Hello, World!'

@app.route('/profile',methods=['GET'])
def profile():
    global data
    data=db.profile(username)
    print(data)
    return render_template('profile.html',data=data)

@app.route('/edit',methods=['GET'])
def edit():
    print(data)
    return render_template('edit.html',data=data)

@app.route('/change',methods=['POST'])
def change():
    global username
    data=request.form.to_dict()
    print(data['name'],data['password'],data['sys_name'])
    db.update(username,data['name'],data['password'],data['sys_name'])
    username=data['name']
    return redirect(url_for('profile'))


@app.route('/sign-out',methods=['GET'])
def sign_out():
    return redirect(url_for('captiveportal'))

@app.route('/connect',methods=['GET'])
def connect():
    return render_template('success.html')
if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000)  
