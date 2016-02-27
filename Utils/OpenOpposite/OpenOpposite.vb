Imports EnvDTE

Module OpenOppositeModule
    Public Function FindItem(ByRef items As ProjectItems, ByRef strFind1 As String, ByRef strFind2 As String) As Boolean

        Dim item As ProjectItem
        For Each item In items
            If item.ProjectItems.Count > 0 Then
                If FindItem(item.ProjectItems, strFind1, strFind2) Then
                    Return True
                End If
            ElseIf UCase(item.Name) = strFind1 Then
                item.Open()
                Return True
            ElseIf UCase(item.Name) = strFind2 Then
                item.Open()
                Return True
            End If
        Next item
        Return False
    End Function

    Public Function OpenOpposite(ByVal Application As Object)

        Dim applicationObject As EnvDTE.DTE
        applicationObject = CType(Application, EnvDTE.DTE)

        Dim strCurrent As String

        strCurrent = UCase(applicationObject.ActiveDocument.Name)

        Dim strFind1 As String
        Dim strFind2 As String

        If strCurrent.EndsWith(".CPP") Then
            strFind1 = strCurrent.Substring(0, strCurrent.Length - 4) + ".H"
        ElseIf strCurrent.EndsWith(".C") Then
            strFind1 = strCurrent.Substring(0, strCurrent.Length - 2) + ".H"
        ElseIf strCurrent.EndsWith(".H") Then
            strFind1 = strCurrent.Substring(0, strCurrent.Length - 2) + ".CPP"
            strFind2 = strCurrent.Substring(0, strCurrent.Length - 2) + ".C"
        End If

        strFind1 = UCase(strFind1)
        strFind2 = UCase(strFind2)
        ' 현재 아이템의 프로젝트를 먼저 찾아본다
        If FindItem(applicationObject.ActiveWindow.Project.ProjectItems, strFind1, strFind2) Then
            Return True
        End If

        Dim i
        For i = 1 To applicationObject.Solution.Projects.Count
            Dim proj As Project
            proj = applicationObject.Solution.Projects.Item(i)
            If proj.Name <> applicationObject.ActiveWindow.Project.Name Then
                If FindItem(proj.ProjectItems, strFind1, strFind2) Then
                    Return True
                End If
            End If
        Next

        Return False
    End Function
End Module

