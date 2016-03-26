using System;
using TerrariaApi.Server;
using Terraria;
using System.IO;
using Google.Protobuf;

namespace protoplugin
{
    [ApiVersion(1,22)]
	public class ProtoPlugin : TerrariaPlugin
	{
		public override string Author => "Tyler W.";
		public override string Description => "Dumps static data from Terraria.Main into protocol buffers for use with paper-tiger.";
		public override bool Enabled => true;
		public override string Name => "ProtoPlugin";
		public override Version Version => new Version(1,0,0,0);

		public ProtoPlugin (Main game) : base(game)
		{
		}

		public override void Initialize ()
		{
            Directory.CreateDirectory("data");

			ServerApi.Hooks.GamePostInitialize.Register(this, (EventArgs args) => {
                DumpTileFlags("data/tile/flags.dat");
			});
		}

		protected void DumpTileFlags(string path)
		{
			tile_flags tf = new tile_flags();

			tf.NumTiles = Main.tileFrameImportant.Length;

			foreach (bool tileFlag in Main.tileFrameImportant) {
				tf.TileFrameImportant.Add(tileFlag);
			}

			foreach (bool tileSolid in Main.tileSolid) {
				tf.TileSolid.Add (tileSolid);
			}

            Directory.CreateDirectory("data/tile");

            using (FileStream fs = File.Create(path)) 
			{
				tf.WriteTo(fs);
			}
		}
	}
}

