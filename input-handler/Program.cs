using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using WindowsInput.Native;
using WindowsInput;
using System.Net.Sockets;
using System.Net;

namespace input_handler
{
  class Program
  {
    static InputSimulator sim = new InputSimulator();

    static void Main(string[] args)
    {
      TcpListener server = null;
      try
      {
        var port = 3030;

        server = new TcpListener(port);
        server.Start();

        var bytes = new byte[256];

        // Enter the listening loop.
        while (true)
        {
          Console.Write("Waiting for a connection... ");

          var client = server.AcceptTcpClient();
          Console.WriteLine("Connected!");

          var stream = client.GetStream();

          int i;
          while ((i = stream.Read(bytes, 0, bytes.Length)) != 0)
          {
            try {
              var split = Encoding.ASCII.GetString(bytes, 0, i).Split('^');
              foreach (var data in split)
              {
                if (data.Length == 0)
                {
                  continue;
                }
                Console.WriteLine("Received: {0}", data);

                VirtualKeyCode key;
                if (data == "A")
                {
                  key = VirtualKeyCode.VK_A;
                }
                else if (data == "B")
                {
                  key = VirtualKeyCode.VK_B;
                }
                else if (data == "#R")
                {
                  key = VirtualKeyCode.RIGHT;
                }
                else if (data == "#L")
                {
                  key = VirtualKeyCode.LEFT;
                }
                else if (data == "#D")
                {
                  key = VirtualKeyCode.DOWN;
                }
                else if (data == "#U")
                {
                  key = VirtualKeyCode.UP;
                }
                else if (data == "R")
                {
                  key = VirtualKeyCode.VK_R;
                }
                else if (data == "L")
                {
                  key = VirtualKeyCode.VK_L;
                }
                else if (data == "X")
                {
                  key = VirtualKeyCode.VK_X;
                }
                else if (data == "Y")
                {
                  key = VirtualKeyCode.VK_Y;
                }
                else if (data == ">")
                {
                  // Select
                  key = VirtualKeyCode.BACK;
                }
                else if (data == "<")
                {
                  // Start
                  key = VirtualKeyCode.RETURN;
                }
                else
                {
                  sim.Keyboard.TextEntry(data);
                  return;
                }

                sim.Keyboard.KeyPress(key);
              }
            }
            catch(Exception e)
            {
              Console.WriteLine("Exception: {0}", e);
            }
          }

          client.Close();
        }
      }
      catch (SocketException e)
      {
        Console.WriteLine("SocketException: {0}", e);
      }
      finally
      {
        server.Stop();
      }
    }
  }
}
