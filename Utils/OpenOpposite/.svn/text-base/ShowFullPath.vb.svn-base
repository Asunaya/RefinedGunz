Imports EnvDTE

Module ShowFullPathModule
    Public Function ShowFullPath(ByVal Application As Object)
        Dim strFullPath As String
        Try
            strFullPath = Application.ActiveDocument.FullName
            'MsgBox(strFullPath)
            Application.StatusBar.Text = strFullPath
        Catch e As System.Exception
            Debug.WriteLine(e.ToString)
        End Try
    End Function
End Module
