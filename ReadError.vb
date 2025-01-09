Imports System
Imports System.IO.Ports
Imports System.Threading

Module Module2

    Sub M()
        SendCommands()

        Console.WriteLine("Press any key to exit...")
        Console.ReadKey()
    End Sub

    Private Sub SendCommands()
        ' Adjust your COM port, baud rate, parity, etc. here.
        Dim port As New SerialPort("COM10", 38400, Parity.None, 8, StopBits.One)
        port.ReadTimeout = 1000   ' optional
        port.WriteTimeout = 1000

        Try
            port.Open()

            ' 1) Send "@3!"
            port.Write("@0!" & vbCr)
            Thread.Sleep(200)  ' ~Pause for 0.2s
            Dim response1 = ReceiveAscii(port)
            Console.WriteLine("Command: @0! -> Reply: " & response1)

            ' 2) Send "@3$"
            port.Write("@0$" & vbCr)
            Thread.Sleep(200)
            Dim response2 = ReceiveAscii(port)
            Console.WriteLine("Command: @0$ -> Reply: " & response2)

        Catch ex As Exception
            Console.WriteLine("Error: " & ex.Message)
        Finally
            If port.IsOpen Then port.Close()
        End Try
    End Sub

    ' This function mimics the old ReceiveAscii() logic:
    ' Wait for data to arrive, wait for the buffer to stabilize, then read everything.
    Private Function ReceiveAscii(port As SerialPort) As String
        Dim beginTime As DateTime = DateTime.Now

        ' 1) Wait up to 0.1s for the first byte
        While port.BytesToRead = 0
            If (DateTime.Now - beginTime).TotalSeconds > 0.1 Then
                ' Timed out waiting for data
                Return String.Empty
            End If
        End While

        ' 2) Wait for the buffer to stop growing
        Dim A As Integer = 0
        Dim B As Integer = port.BytesToRead
        While A <> B
            A = B
            Thread.Sleep(20)  ' ~Pause 0.02s
            B = port.BytesToRead
        End While

        ' 3) Read what's in the buffer
        Return port.ReadExisting()
    End Function

End Module
