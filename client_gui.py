"""
The client gui application
"""

#import wxversion
#wxversion.select("3.0")
import wx

import socket
import sys

"""
The main panel that gives the login/signup information
"""
class MainPanel(wx.Frame):
    """
    The constructor for the main panel that displays the necessary widgets
    """
    def __init__(self, parent, title):

        super(MainPanel, self).__init__(parent, title=title, 
            size=(500, 500))
        panel = wx.Panel(self)

	#self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	'''host = socket.gethostbyname(sys.argv[1])
	mPort = 8777
	sPort = 8778
	mAddress = (host,mPort)
	sAddress = (host,sPort)'''

	'''try:
		self.sock.connect(mAddress)
	except:
		self.sock.connect(sAddress)'''
	
        hbox = wx.BoxSizer(wx.HORIZONTAL)

        fgs = wx.FlexGridSizer(3, 2, 9, 25)

        userSizer = wx.StaticText(panel, label="User name", pos = (140, 250))
        passSizer = wx.StaticText(panel, label="Password", pos = (140, 300))

	png = wx.Image("./index.png", wx.BITMAP_TYPE_ANY).ConvertToBitmap()
	wx.StaticBitmap(panel, 1, png, (125, 5), (png.GetWidth(), png.GetHeight()))	

	self.loginbut = wx.Button(panel, label='Login', pos=(50, 400))

	self.signupbut = wx.Button(panel, label='Sign up', pos=(350, 400))

	self.tc1 = wx.TextCtrl(panel, style = wx.TE_PROCESS_ENTER, pos = (250, 250))
	bsizer = wx.BoxSizer()
	bsizer.Add(self.tc1, 1, wx.EXPAND)

        self.tc2 = wx.TextCtrl(panel, style=wx.TE_PASSWORD|wx.TE_PROCESS_ENTER, pos = (250,300))

	self.loginbut.Bind(wx.EVT_BUTTON, self.OnLogin)
	self.signupbut.Bind(wx.EVT_BUTTON, self.OnSignup)

        '''fgs.AddMany([(userSizer), (self.tc1, 1, wx.EXPAND), (passSizer), 
            (self.tc2, 1, wx.EXPAND)])'''

        fgs.AddGrowableRow(2, 1)
        fgs.AddGrowableCol(1, 1)

        hbox.Add(fgs, proportion=1, flag=wx.ALL|wx.EXPAND, border=15)
        panel.SetSizer(hbox)
        self.Centre()
        self.Show()  

    def OnLogin(self, e):
	code = '2'
        self.OnConnectInit(code)
        
    def OnSignup(self, e):
	code = '1'
	self.OnConnectInit(code)        

    def OnConnectInit(self,code): 
	
	print 'User name: ', self.tc1.GetValue()
	print 'Password: ', self.tc2.GetValue()
	serverFormat = str(code)+':'+self.tc1.GetValue()+':'+self.tc2.GetValue()
	
	print 'Server Format: ', serverFormat

	sock.recv(1024)
        sock.send(serverFormat)
        
	errCode = sock.recv(1024)
	print "Error code : ", errCode
	#errCode = "USER_NAME_ERROR"
	if(errCode == "USER_NAME_ERROR" or errCode == "PASSWORD_ERROR" or errCode == "USER_NAME_TAKEN" or errCode == "ALREADY_LOGGED_IN"):
		self.loginbut.Bind(wx.EVT_BUTTON, self.OnLogin)
		self.signupbut.Bind(wx.EVT_BUTTON, self.OnSignup)
	
	else:
		print "Success going to second panel"
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
            size=(550, 500))
        pnl = wx.Panel(self)

        hbox = wx.BoxSizer(wx.HORIZONTAL)

	print "Second panel"

        fgs = wx.FlexGridSizer(3, 2, 9, 25)

	promptSizer = wx.StaticText(pnl, -1,
		label = "Enter the word that pops up in your mind on reading", pos = (20, 10))
	font = wx.Font(15,wx.DECORATIVE, wx.ITALIC, wx.NORMAL)
	promptSizer.SetFont(font)
	promptSizer.SetForegroundColour((0,0,255))

	response = sock.recv(1024)

	words = response.split("_")

	word = words[0]

	print "Word is" + word
	
	wordSizer = wx.StaticText(pnl, -1,label = word, pos = (220, 50))
	font = wx.Font(25,wx.DECORATIVE, wx.ITALIC, wx.NORMAL)
	wordSizer.SetFont(font)
	wordSizer.SetForegroundColour((255,0,0))
	
	enterbut = wx.Button(pnl, label='Enter', pos=(215, 200), size = (100,50))

	self.word = wx.TextCtrl(pnl, style = wx.TE_PROCESS_ENTER, pos = (175,100), 
	size = (200,100))


	enterbut.Bind(wx.EVT_BUTTON, self.OnButtonPress)
	
	timer = wx.Timer(pnl)
	timer.Start(100)	

	
	hbox.Add(fgs, proportion=1, flag=wx.ALL|wx.EXPAND, border=15)
        pnl.SetSizer(hbox)
        self.Centre()
        self.Show()

    def OnButtonPress(self, e):
	print "Entered word: ", self.word.GetValue()
	self.Close(True)


if __name__ == '__main__':

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
  
    app = wx.App()
    MainPanel(None, title='Mind Sync')
    app.MainLoop()
