Imports System
Imports System.IO.Ports
Imports System.Threading

Module Module1
    Sub Main()
        Dim port As New SerialPort("COM10", 38400, Parity.None, 8, StopBits.One)

        Try
            port.Open()

            ' Each command from your VB example, sending a CR (Chr(13)) at the end:
            port.Write("@0A1_100000" & Chr(13))
            Thread.Sleep(500)

            port.Write("@0B1_1000" & Chr(13))
            Thread.Sleep(500)

            port.Write("@0M1_4000" & Chr(13))
            Thread.Sleep(500)

            port.Write("@0N1_500" & Chr(13))
            Thread.Sleep(500)

            port.Write("@0T1_100" & Chr(13))
            Thread.Sleep(500)

            port.Write("@0R8" & Chr(13))
            Thread.Sleep(500)

            port.Write("@0+" & Chr(13))
            Thread.Sleep(500)

            port.Write("@0G1" & Chr(13))
            Thread.Sleep(500)

            Console.WriteLine("Commands sent. Press any key to exit.")
            Console.ReadKey()

        Catch ex As Exception
            Console.WriteLine("ERROR: " & ex.Message)
        Finally
            If port.IsOpen Then
                port.Close()
            End If
        End Try
    End Sub
End Module

