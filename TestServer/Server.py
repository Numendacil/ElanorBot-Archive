from tokenize import group
from flask import Flask, request
from flask_sock import Sock
import json
app = Flask(__name__)
sockets = Sock(app)

VerifyKey = '12345'
SessionKey = "abcdef"
qq = 123456789

Group = 88888888
Sender = 11111111


@app.route('/verify', methods = ['POST'])
def verify():
	req = request.get_json(force=True)
	resp = {}

	if req.get("verifyKey", "") == VerifyKey:
		resp["code"] = 0
		resp["session"] = SessionKey
	else:
		resp["code"] = -1
		resp["msg"] = ""
	return json.dumps(resp)

@app.route('/bind', methods = ['POST'])
def bind():
	req = request.get_json(force=True)
	resp = {}

	if req.get("sessionKey", "") == SessionKey and req.get("qq", 0) == qq:
		if req.get("qq", 0) == qq:
			resp["code"] = 0
			resp["msg"] = "success"
		else:
			resp["code"] = 2
			resp["msg"] = ""
	else:
		resp["code"] = -1
		resp["msg"] = ""
	return json.dumps(resp)

@app.route('/release', methods = ['POST'])
def release():
	req = request.get_json(force=True)
	resp = {}

	if req.get("sessionKey", "") == SessionKey:
		if req.get("qq", 0) == qq:
			resp["code"] = 0
			resp["msg"] = "success"
		else:
			resp["code"] = 2
			resp["msg"] = ""
	else:
		resp["code"] = -1
		resp["msg"] = ""
	return json.dumps(resp)

@app.route('/sessionInfo', methods = ['GET'])
def sessionInfo():
	resp = {}
	if request.args.get("sessionKey", "") == SessionKey:
		resp["code"] = 0
		resp["msg"] = ""
		resp["data"] = {"sessionKey": SessionKey, "qq": {"id": qq, "nickname": "www", "remark": ""}}
	else:
		resp["code"] = -1
		resp["msg"] = ""
	return json.dumps(resp)

@app.route('/about', methods = ['GET'])
def about():
	resp = {}
	resp["code"] = 0
	resp["msg"] = ""
	resp["data"] = {"version": "2.4.0"}
	return json.dumps(resp)

@app.route('/groupConfig', methods = ['GET'])
def groupCfg():
	resp = {}
	if request.args.get("sessionKey", "") == SessionKey and request.args.get("target", 0) == Group:
		resp["name"] = "Group"
		resp["announcement"] = "announcement"
		resp["confessTalk"] = True
		resp["allowMemberInvite"] = True
		resp["autoApprove"] = False
		resp["anonymousChat"] = False
	else:
		resp["code"] = -1
		resp["msg"] = ""
	return json.dumps(resp)

@app.route('/memberInfo', methods = ['GET'])
def memberInfo():
	resp = {}
	if request.args.get("sessionKey", "") == SessionKey and request.args.get("target", 0) == Group:
		if request.args.get("memberId", 0) == Sender:
			resp["id"] = Sender
			resp["memberName"] = "Me"
			resp["specialTitle"] = ""
			resp["permission"] = "OWNER"
			resp["joinTimestamp"] = 0
			resp["lastSpeakTimestamp"] = 0
			resp["muteTimeRemaining"] = 0
			resp["group"] = {"id": Group, "name":"Group", "permission": "ADMINISTRATOR"}
		elif request.args.get("memberId", 0) == qq:
			resp["id"] = qq
			resp["memberName"] = "Bot"
			resp["specialTitle"] = ""
			resp["permission"] = "ADMINISTRATOR"
			resp["joinTimestamp"] = 0
			resp["lastSpeakTimestamp"] = 0
			resp["muteTimeRemaining"] = 0
			resp["group"] = {"id": Group, "name":"Group", "permission": "ADMINISTRATOR"}
		else:
			resp["code"] = -1
			resp["msg"] = "Not found"

	else:
		resp["code"] = -1
		resp["msg"] = ""
	return json.dumps(resp)

@app.errorhandler(500)
def pageNotFound(error):
	resp = {}
	resp["code"] = -1
	return json.dumps(resp)


@app.route('/<path:path>', methods = ['GET'])
def catch_get(path):
	resp = {}
	resp["code"] = -1
	resp["msg"] = ""
	return json.dumps(resp)

@app.route('/<path:path>', methods = ['POST'])
def catch_post(path):
	print(f"path: {path}, data: {request.get_json(force=True)}")
	resp = {}
	resp["code"] = 0
	resp["msg"] = "success"
	resp["messageId"] = 0
	return json.dumps(resp)
@sockets.route('/all')
def websocket_route(ws):
	try:
		while True:
			data = input('> ')


			msg = {}
			msg["type"] = "GroupMessage"
			sender = {}
			sender["id"] = Sender
			sender["memberName"] = "Me"
			sender["specialTitle"] = ""
			sender["permission"] = "OWNER"
			sender["joinTimestamp"] = 0
			sender["lastSpeakTimestamp"] = 0
			sender["muteTimeRemaining"] = 0
			sender["group"] = {"id": Group, "name":"Group", "permission": "ADMINISTRATOR"}
			msg["sender"] = sender
			msg["messageChain"] = [{"type": "Plain", "text": data}]

			ws.send(json.dumps({"syncId": "1", "data": msg}))
	except :
		pass
	return ''
		

if __name__ == '__main__':
	app.run(host="localhost", port=10080)