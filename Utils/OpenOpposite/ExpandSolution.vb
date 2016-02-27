Imports EnvDTE

Module ExpandSolutionModule
    Public Function ExpandProject(ByVal projItems As ProjectItems)
        Dim item As ProjectItem
        For Each item In projItems
            If (item.ProjectItems.Count > 0) Then
                item.ExpandView()
                ExpandProject(item.ProjectItems)
            End If
        Next
    End Function

    Public Function ExpandSolution(ByVal Application As Object, ByVal bExpand As Boolean)
        If Application.ActiveSolutionProjects.Length > 0 Then
            ExpandProject(Application.ActiveSolutionProjects(0).ProjectItems)
        Else
            Dim project As Project
            For Each project In Application.Solution.Projects
                ExpandProject(project.ProjectItems)
            Next
        End If
    End Function
End Module
