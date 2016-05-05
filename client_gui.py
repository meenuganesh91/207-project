#The client gui application
import wxversion
wxversion.select("3.0")
import wx

#The main panel
class MainPanel(wx.Frame):
  
    def __init__(self, parent, title):
        super(MainPanel, self).__init__(parent, title=title, 
            size=(300, 250))
        panel = wx.Panel(self)

        hbox = wx.BoxSizer(wx.HORIZONTAL)

        fgs = wx.FlexGridSizer(3, 2, 9, 25)

        userSizer = wx.StaticText(panel, label="User name")
        passSizer = wx.StaticText(panel, label="Password")
	
	loginbut = wx.Button(panel, label='Login', pos=(20, 100))

	signupbut = wx.Button(panel, label='Sign up', pos=(190, 100))

	self.tc1 = wx.TextCtrl(panel, style = wx.TE_PROCESS_ENTER)

        self.tc2 = wx.TextCtrl(panel, style=wx.TE_PASSWORD|wx.TE_PROCESS_ENTER)

	loginbut.Bind(wx.EVT_BUTTON, self.OnConnectInit)
	signupbut.Bind(wx.EVT_BUTTON, self.OnConnectInit)

        fgs.AddMany([(userSizer), (self.tc1, 1, wx.EXPAND), (passSizer), 
            (self.tc2, 1, wx.EXPAND)])

        fgs.AddGrowableRow(2, 1)
        fgs.AddGrowableCol(1, 1)

        hbox.Add(fgs, proportion=1, flag=wx.ALL|wx.EXPAND, border=15)
        panel.SetSizer(hbox)
        self.Centre()
        self.Show()     
        
    def OnConnectInit(self, e): 
	print 'User name: ', self.tc1.GetValue()
	print 'Password: ', self.tc2.GetValue()
	serverFormat = self.tc1.GetValue()+':'+self.tc2.GetValue()
	print 'Server Format: ', serverFormat
        print "You are now logged in."
	#app2 = wx.App()
	app2 = SecondPanel(None, title = 'Mind Sync')
	self.Hide()
	app2.Show()
	#app2.MainLoop()
        self.Close(True) 

#The second panel to be shown
class SecondPanel(wx.Frame):

    def __init__(self, parent, title):
        super(SecondPanel, self).__init__(parent, title=title, 
            size=(300, 250))
        pnl = wx.Panel(self)

        hbox = wx.BoxSizer(wx.HORIZONTAL)

        fgs = wx.FlexGridSizer(3, 2, 9, 25)
	wordSizer = wx.StaticText(pnl, label="Apple")
	enterbut = wx.Button(pnl, label='Enter', pos=(100, 100))

	self.word = wx.TextCtrl(pnl, style = wx.TE_PROCESS_ENTER)

	enterbut.Bind(wx.EVT_BUTTON, self.OnButtonPress)

	fgs.AddMany([(wordSizer), (self.word, 1, wx.EXPAND)])

	hbox.Add(fgs, proportion=1, flag=wx.ALL|wx.EXPAND, border=15)
        pnl.SetSizer(hbox)
        self.Centre()
        self.Show()

    def OnButtonPress(self, e):
	print "Entered word: ", self.word.GetValue()
	self.Close(True)


if __name__ == '__main__':
  
    app = wx.App()
    MainPanel(None, title='Mind Sync')
    app.MainLoop()
