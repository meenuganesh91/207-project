#!/usr/bin/python

"""
The client gui application
"""

#import wxversion
#wxversion.select("3.0")
import wx
import time
import socket
import sys

USER_STATS = ""
sock = 0

"""
The main panel that gives the login/signup information
"""
class MainPanel(wx.Frame):
    """
    The constructor for the main panel that displays the necessary widgets
    """
    def __init__(self, parent, title):

        super(MainPanel, self).__init__(parent, title=title, 
            size=(500, 700))
        self.panel = wx.Panel(self)
	
        hbox = wx.BoxSizer(wx.HORIZONTAL)

        fgs = wx.FlexGridSizer(3, 2, 9, 25)

        userSizer = wx.StaticText(self.panel, label="User name", pos = (140, 250))
        passSizer = wx.StaticText(self.panel, label="Password", pos = (140, 300))

	png = wx.Image("./index.png", wx.BITMAP_TYPE_ANY).ConvertToBitmap()
	wx.StaticBitmap(self.panel, 1, png, (125, 5), (png.GetWidth(), png.GetHeight()))	

	self.loginbut = wx.Button(self.panel, label='Login', pos=(50, 400))

	self.signupbut = wx.Button(self.panel, label='Sign up', pos=(350, 400))

	self.tc1 = wx.TextCtrl(self.panel, style = wx.TE_PROCESS_ENTER, pos = (250, 250))
	bsizer = wx.BoxSizer()
	bsizer.Add(self.tc1, 1, wx.EXPAND)

        self.tc2 = wx.TextCtrl(self.panel, style=wx.TE_PASSWORD|wx.TE_PROCESS_ENTER, pos = (250,300))

	self.errorMsg = wx.StaticText(self.panel, label="Welcome to Mind Sync", 
				pos = (150, 350))
	self.errorMsg.SetForegroundColour((255,0,0))

	self.loginbut.Bind(wx.EVT_BUTTON, self.OnLogin)
	self.signupbut.Bind(wx.EVT_BUTTON, self.OnSignup)

        fgs.AddGrowableRow(2, 1)
        fgs.AddGrowableCol(1, 1)

        hbox.Add(fgs, proportion=1, flag=wx.ALL|wx.EXPAND, border=15)
        self.panel.SetSizer(hbox)
        self.Centre()
        self.Show()  

    def OnLogin(self, e):
	code = '2'
	blank = ""
	print "Trying to login "
	self.errorMsg.SetLabel("Welcome to Mind Sync")
	global sock
	sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	host = socket.gethostbyname(sys.argv[1])
	mPort = 8777
	sPort = 8778
	mAddress = (host,mPort)
	sAddress = (host,sPort)
	try:
		sock.connect(mAddress)
    	except:
		sock.connect(sAddress)
  
        self.OnConnectInit(code)
        
    def OnSignup(self, e):
	global sock
	blank = ""
	self.errorMsg.SetLabel("Welcome to Mind Sync")
	sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	sock.settimeout(30.0)
	host = socket.gethostbyname(sys.argv[1])
	mPort = 8777
	sPort = 8778
	mAddress = (host,mPort)
	sAddress = (host,sPort)
	try:
		sock.connect(mAddress)
    	except:
		sock.connect(sAddress)
  
	code = '1'
	self.OnConnectInit(code)        

    def OnConnectInit(self,code): 
	
	print 'User name: ', self.tc1.GetValue()
	print 'Password: ', self.tc2.GetValue()
	serverFormat = str(code)+':'+self.tc1.GetValue()+':'+self.tc2.GetValue()
	
	print 'Server Format: ', serverFormat
	if(self.tc1.GetValue() == "" or self.tc2.GetValue() == ""):
		errorMsg = wx.StaticText(self.panel, label="User name or password cannot be empty", 
			pos = (150, 350))
		errorMsg.SetForegroundColour((255,0,0))	
	sock.recv(1024)

        sock.send(serverFormat)

        
	errCode = sock.recv(1024)
	print "Error code : ", errCode
	
	if(errCode == "USER_NAME_ERROR" or errCode == "PASSWORD_ERROR" or errCode == "USER_NAME_TAKEN" or errCode == "USER_ALREADY_LOGGED"):
		if(errCode == "USER_NAME_ERROR" or errCode == "PASSWORD_ERROR"):
			err = "Wrong User name or password"
			self.errorMsg.SetLabel(err)

		elif(errCode == "USER_NAME_TAKEN"):
			err = "User name taken"
			self.errorMsg.SetLabel(err)

		elif (errCode == "USER_ALREADY_LOGGED"):
			err = "User already logged in"
			self.errorMsg.SetLabel(err)
			

		sock.close()
		self.loginbut.Bind(wx.EVT_BUTTON, self.OnLogin)
		self.signupbut.Bind(wx.EVT_BUTTON, self.OnSignup)

	else:   
		print "Success going to second panel"
		global USER_STATS 
		USER_STATS = sock.recv(1024)
		print "Stats ", USER_STATS

		app2 = SecondPanel(None, title = 'Mind Sync')
		self.Hide()
		app2.Show()
        	self.Close(True) 


"""
The second panel that shows the word and the other player's response and takes in the input 
"""
class SecondPanel(wx.Frame):

    def __init__(self, parent, title):
        super(SecondPanel, self).__init__(parent, title=title, 
            size=(550, 700))
        self.pnl = wx.Panel(self)

	#self.pnl.SetBackgroundColour((237,125,49))
        hbox = wx.BoxSizer(wx.HORIZONTAL)

	print "Second panel"

        fgs = wx.FlexGridSizer(3, 2, 9, 25)


	png1 = wx.Image("./index.png", wx.BITMAP_TYPE_ANY).ConvertToBitmap()
	wx.StaticBitmap(self.pnl, 1, png1, (5, 5), (png1.GetWidth(), png1.GetHeight()))

	promptSizer = wx.StaticText(self.pnl, -1,
		label = "Enter the word that pops up in your mind on reading", pos = (20, 240))
	font = wx.Font(15,wx.DECORATIVE, wx.ITALIC, wx.NORMAL)
	promptSizer.SetFont(font)
	promptSizer.SetForegroundColour((0,0,255))

	stats = USER_STATS.split('_')
	print "Testing " + USER_STATS

	font = wx.Font(10,wx.DECORATIVE, wx.ITALIC, wx.BOLD)

	scoreMsg1 = wx.StaticText(self.pnl, label="Total score:   "+str(stats[1]), pos = (350,85))
	scoreMsg2 = wx.StaticText(self.pnl, label="Highest score: "+str(stats[2]),pos=(350,105))
	scoreMsg3 = wx.StaticText(self.pnl, label="Lowest score:  "+str(stats[3]), pos=(350,125))

	scoreMsg1.SetFont(font)

	scoreMsg2.SetFont(font)
	
	scoreMsg3.SetFont(font)

	response = sock.recv(1024)

	words = response.split("_")

	wrd = words[0]

	print "Word: " + wrd
	
	self.wordSizer = wx.StaticText(self.pnl, -1,label = wrd, pos = (220, 300))
	font = wx.Font(25,wx.DECORATIVE, wx.ITALIC, wx.NORMAL)
	self.wordSizer.SetFont(font)
	self.wordSizer.SetForegroundColour((255,0,0))
	
	enterbut = wx.Button(self.pnl, label='Enter', pos=(100, 500), size = (100,50))

	exitbut = wx.Button(self.pnl, label='End Game', pos=(300, 500), size = (100,50))

	self.word = wx.TextCtrl(self.pnl, style = wx.TE_PROCESS_ENTER, pos = (175,400), 
	size = (200,50))
	
	hbox.Add(fgs, proportion=1, flag=wx.ALL|wx.EXPAND, border=15)
        self.pnl.SetSizer(hbox)
        self.Centre()
        self.Show()
	
	scoreString = "Score: "+words[1]
        self.scoreSizer = wx.StaticText(self.pnl, -1,
        label = scoreString , pos = (300, 600))
        scorefont = wx.Font(15,wx.DECORATIVE, wx.NORMAL, wx.NORMAL)
        self.scoreSizer.SetFont(scorefont)

        responseString = words[2]+"'s response: "+words[3]
        self.responseSizer = wx.StaticText(self.pnl, -1,
        label = responseString , pos = (300, 570))
        responsefont = wx.Font(15,wx.DECORATIVE, wx.NORMAL, wx.NORMAL)
        self.responseSizer.SetFont(responsefont)

		
	'''self.connectionErrorMsg = wx.StaticText(self.pnl,-1, label="Connected!!", pos = (300, 550))		
	connectionErrorFont = wx.Font(15,wx.DECORATIVE, wx.ITALIC, wx.NORMAL)
	self.connectionErrorMsg.SetFont(connectionErrorFont)
	self.connectionErrorMsg.SetForegroundColour((255,0,0))
	
	self.gameEndErrorMsg = wx.StaticText(self.pnl, -1, label="Keep Playing!!!", pos = (15, 550))
	gameEndFont = wx.Font(15,wx.DECORATIVE, wx.ITALIC, wx.NORMAL)
	self.gameEndErrorMsg.SetFont(gameEndFont)
	self.gameEndErrorMsg.SetForegroundColour((255,0,0))'''

	enterbut.Bind(wx.EVT_BUTTON, self.OnEnterPress)

	exitbut.Bind(wx.EVT_BUTTON, self.OnExitPress)


	self.timerSizer = wx.StaticText(self.pnl, -1,label = "Timer : ", pos = (5,600))
	timerfont = wx.Font(15,wx.DECORATIVE, wx.NORMAL, wx.NORMAL)
	self.timerSizer.SetFont(timerfont)
	
        self.timersz = wx.StaticText(self.pnl, -1, label="60",pos = (80,600))
        self.timersz.SetFont(timerfont)

        self.counter = 60
        self.timer = wx.Timer(self)
        self.Bind(wx.EVT_TIMER, self.onTimer, self.timer)
        self.timer.Start(1000)

    def onTimer(self, e):

        self.counter -= 1
        msg = "%s " % self.counter
        self.timersz.SetLabel(msg)
        if self.counter <= 0:
            self.timer.Stop()   # stop the timer
	    no_responce_msg = "NO_RESPONSE"
	    sock.send(no_responce_msg)
	    print "Entered word: "+ no_responce_msg

	


    def OnEnterPress(self, e):
	print "Entered word: ", self.word.GetValue()
	sock.send(self.word.GetValue())
	self.word.Clear()
	print "receiving... "
	self.timer.Stop()
	self.counter = 61 
        self.timer = wx.Timer(self)
        self.Bind(wx.EVT_TIMER, self.onTimer, self.timer)
        self.timer.Start(1000)
	try:
		response = sock.recv(1024)
	except:
       		print "receive error or timedout"	
		self.connectionErrorMsg.SetLabel("Connection error")
		self.connectErrorMsg.SetForegroundColour((255,0,0))
		time.sleep(5)
		sys.exit(0)

	words = response.split("_")
	wrd = words[0]
	print "receive end" + wrd
	
        #wrd = "GAMEEND"	
	if (wrd == "GAMEEND"):
       		print "received GAMEEND"	
		'''gameErrormsg = "Your partner has closed the game, the game will exit now!!!"
		self.gameEndErrorMsg.SetLabel(gameErrormsg)
		gameEndFont = wx.Font(15,wx.DECORATIVE, wx.ITALIC, wx.NORMAL)
		self.gameEndErrorMsg.SetFont(gameEndFont)
		self.gameEndErrorMsg.SetForegroundColour((255,0,0))
		#	time.sleep(5)
		self.pnl.Destroy()
	        sys.exit(0)'''
		response = sock.recv(1024)
		words = response.split("_")
		wrd = words[0]
		print "receive end" + wrd
	
	self.wordSizer.SetLabel(wrd)
	font = wx.Font(25,wx.DECORATIVE, wx.ITALIC, wx.NORMAL)
	self.wordSizer.SetFont(font)
	self.wordSizer.SetForegroundColour((255,0,0))
	print "Word is" + wrd
	
	self.scoreSizer.SetLabel("Score: "+words[1])
        scorefont = wx.Font(15,wx.DECORATIVE, wx.NORMAL, wx.NORMAL)
        self.scoreSizer.SetFont(scorefont)
        self.scoreSizer.SetForegroundColour((0,0,0))

        self.responseSizer.SetLabel(words[2]+"'s response: "+words[3])
        responsefont = wx.Font(15,wx.DECORATIVE, wx.NORMAL, wx.NORMAL)
        self.responseSizer.SetFont(responsefont)
        self.responseSizer.SetForegroundColour((0,0,0))


    def OnExitPress(self, e):
	print "Exiting"
	try:
		sock.close()
	except:
		sock.shutdown(socket.SHUT_RDWR)
		sock.close()
	finally:
		self.Close(True)

if __name__ == '__main__':
    app = wx.App()
    MainPanel(None, title='Mind Sync')
    app.MainLoop()
    
