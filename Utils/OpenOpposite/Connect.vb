Imports Microsoft.Office.Core
imports Extensibility
imports System.Runtime.InteropServices
Imports EnvDTE

#Region " Read me for Add-in installation and setup information. "
' When run, the Add-in wizard prepared the registry for the Add-in.
' At a later time, if the Add-in becomes unavailable for reasons such as:
'   1) You moved this project to a computer other than which is was originally created on.
'   2) You chose 'Yes' when presented with a message asking if you wish to remove the Add-in.
'   3) Registry corruption.
' you will need to re-register the Add-in by building the OpenOppositeSetup project 
' by right clicking the project in the Solution Explorer, then choosing install.
#End Region

<GuidAttribute("375F767F-7850-4803-A283-8181D1A2A379"), ProgIdAttribute("OpenOpposite.Connect")> _
Public Class Connect

    Implements Extensibility.IDTExtensibility2
    Implements IDTCommandTarget

    Dim applicationObject As EnvDTE.DTE
    Dim addInInstance As EnvDTE.AddIn

    Public Sub OnBeginShutdown(ByRef custom As System.Array) Implements Extensibility.IDTExtensibility2.OnBeginShutdown
    End Sub

    Public Sub OnAddInsUpdate(ByRef custom As System.Array) Implements Extensibility.IDTExtensibility2.OnAddInsUpdate
    End Sub

    Public Sub OnStartupComplete(ByRef custom As System.Array) Implements Extensibility.IDTExtensibility2.OnStartupComplete
    End Sub

    Public Sub OnDisconnection(ByVal RemoveMode As Extensibility.ext_DisconnectMode, ByRef custom As System.Array) Implements Extensibility.IDTExtensibility2.OnDisconnection
    End Sub

    Public Sub OnConnection(ByVal application As Object, ByVal connectMode As Extensibility.ext_ConnectMode, ByVal addInInst As Object, ByRef custom As System.Array) Implements Extensibility.IDTExtensibility2.OnConnection

        applicationObject = CType(application, EnvDTE.DTE)
        addInInstance = CType(addInInst, EnvDTE.AddIn)

        If connectMode = Extensibility.ext_ConnectMode.ext_cm_UISetup Then
            Dim objAddIn As AddIn = CType(addInInst, AddIn)
            Dim CommandObj As Command

            ' When run, the Add-in wizard prepared the registry for the Add-in.
            ' At a later time, the Add-in or its commands may become unavailable for reasons such as:
            '   1) You moved this project to a computer other than which is was originally created on.
            '   2) You chose 'Yes' when presented with a message asking if you wish to remove the Add-in.
            '   3) You add new commands or modify commands already defined.
            ' You will need to re-register the Add-in by building the OpenOppositeSetup project,
            ' right-clicking the project in the Solution Explorer, and then choosing install.
            ' Alternatively, you could execute the ReCreateCommands.reg file the Add-in Wizard generated in
            ' the project directory, or run 'devenv /setup' from a command prompt.
            Try
                CommandObj = applicationObject.Commands.AddNamedCommand(objAddIn, "OpenOpposite", "", "Executes the command for OpenOpposite", True, 59, Nothing, 1 + 2)  '1+2 == vsCommandStatusSupported+vsCommandStatusEnabled
                CommandObj = applicationObject.Commands.AddNamedCommand(objAddIn, "ShowFullPath", "", "Show Full Path", True, 60, Nothing, 1 + 2)  '1+2 == vsCommandStatusSupported+vsCommandStatusEnabled
                CommandObj = applicationObject.Commands.AddNamedCommand(objAddIn, "ExpandSolution", "", "Expand Solution", True, 61, Nothing, 1 + 2)  '1+2 == vsCommandStatusSupported+vsCommandStatusEnabled

                'CommandObj.AddControl(applicationObject.CommandBars.Item("Tools"))



                ' Before running, you must add a reference to the Office 
                ' typelib to gain access to the CommandBar object. Also, for this 
                ' example to work correctly, there should be an add-in available 
                ' in the Visual Studio .NET environment.
                Dim cmds As Commands
                Dim cmdobj As Command
                'Dim customin, customout As Object
                Dim cmdbarobj As CommandBar
                Dim colAddins As AddIns

                ' Set references.
                'colAddins = applicationObject.DTE.AddIns()
                cmds = applicationObject.DTE.Commands
                'cmdobj = cmds.Item("File.NewFile")

                ' Create a toolbar and add the File.NewFile command to it.
                cmdbarobj = cmds.AddCommandBar("OpenOpposite", vsCommandBarType.vsCommandBarTypeToolbar)
                CommandObj.AddControl(cmdbarobj)

            Catch e As System.Exception
            End Try

        End If


    End Sub

    Public Sub Exec(ByVal cmdName As String, ByVal executeOption As vsCommandExecOption, ByRef varIn As Object, ByRef varOut As Object, ByRef handled As Boolean) Implements IDTCommandTarget.Exec
        handled = False
        If (executeOption = vsCommandExecOption.vsCommandExecOptionDoDefault) Then
            If cmdName = "OpenOpposite.Connect.OpenOpposite" Then
                handled = True
                OpenOpposite(applicationObject)
                Exit Sub
            ElseIf cmdName = "OpenOpposite.Connect.ShowFullPath" Then
                handled = True
                ShowFullPath(applicationObject)
                Exit Sub
            ElseIf cmdName = "OpenOpposite.Connect.ExpandSolution" Then
                handled = True
                ExpandSolution(applicationObject, True)
                Exit Sub
            End If
        End If
    End Sub

    Public Sub QueryStatus(ByVal cmdName As String, ByVal neededText As vsCommandStatusTextWanted, ByRef statusOption As vsCommandStatus, ByRef commandText As Object) Implements IDTCommandTarget.QueryStatus
        If neededText = EnvDTE.vsCommandStatusTextWanted.vsCommandStatusTextWantedNone Then
            If cmdName = "OpenOpposite.Connect.OpenOpposite" Then
                statusOption = CType(vsCommandStatus.vsCommandStatusEnabled + vsCommandStatus.vsCommandStatusSupported, vsCommandStatus)
            ElseIf cmdName = "OpenOpposite.Connect.ShowFullPath" Then
                statusOption = CType(vsCommandStatus.vsCommandStatusEnabled + vsCommandStatus.vsCommandStatusSupported, vsCommandStatus)
            ElseIf cmdName = "OpenOpposite.Connect.ExpandSolution" Then
                statusOption = CType(vsCommandStatus.vsCommandStatusEnabled + vsCommandStatus.vsCommandStatusSupported, vsCommandStatus)
            Else
                statusOption = vsCommandStatus.vsCommandStatusUnsupported
            End If
        End If
    End Sub
End Class

