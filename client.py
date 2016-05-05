
import wxversion
wxversion.select("3.0")
import wx


class Panel(wx.Frame):
  
    def __init__(self, parent, title):
        super(Panel, self).__init__(parent, title=title, 
            size=(300, 250))
        panel = wx.Panel(self)

        hbox = wx.BoxSizer(wx.HORIZONTAL)

        fgs = wx.FlexGridSizer(3, 2, 9, 25)

        userSizer = wx.StaticText(panel, label="User name")
        passSizer = wx.StaticText(panel, label="Password")
	
	loginbut = wx.Button(panel, label='Login', pos=(20, 100))

	signupbut = wx.Button(panel, label='Sign up', pos=(190, 100))

        tc1 = wx.TextCtrl(panel)
        tc2 = wx.TextCtrl(panel)
	'''self.password = wx.TextCtrl(self, style=wx.TE_PASSWORD|wx.TE_PROCESS_ENTER)
        self.password.Bind(wx.EVT_TEXT_ENTER, self.OnConnectInit)
        hbox.Add(self.password, 0, wx.ALL, 5)'''

	loginbut.Bind(wx.EVT_BUTTON, self.OnConnectInit)
	signupbut.Bind(wx.EVT_BUTTON, self.OnConnectInit)

        fgs.AddMany([(userSizer), (tc1, 1, wx.EXPAND), (passSizer), 
            (tc2, 1, wx.EXPAND)])

        fgs.AddGrowableRow(2, 1)
        fgs.AddGrowableCol(1, 1)

        hbox.Add(fgs, proportion=1, flag=wx.ALL|wx.EXPAND, border=15)
        panel.SetSizer(hbox)
        self.Centre()
        self.Show()     
        

    def OnConnectInit(self, e):
        print "You are now logged in."
	#usrNm = self.tc1.GetValue()
	#psWrd = self.password.GetValue()
	#print usrNm
	#print psWrd
        self.Close(True) 


if __name__ == '__main__':
  
    app = wx.App()
    Panel(None, title='Mind Sync')
    app.MainLoop()
