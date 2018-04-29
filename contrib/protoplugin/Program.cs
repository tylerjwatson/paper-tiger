using System;
using System.IO;
using System.Reflection;
using Steamworks;

namespace protoplugin
{
    class Program
    {
        static void Main(string[] args)
        {
            SteamUtils.IsOverlayEnabled();
            try
            {
            Terraria.Program.ForceLoadAssembly(typeof(Terraria.Program).Assembly, true);
            }
            catch (ReflectionTypeLoadException rte)
            {
                Console.WriteLine(rte);
                foreach (var ex in rte.LoaderExceptions)
                {
                    Console.WriteLine(ex);
                }
            }

            Directory.CreateDirectory("data");

            ProtoPlugin.DumpTileFlags("data/tile/flags.dat");
        }
    }
}
