using System;
using Terraria;
using System.IO;
using Google.Protobuf;

namespace protoplugin
{
	public static class ProtoPlugin
	{
		public static void DumpTileFlags(string path)
		{
			Main game = new Main();
			game.Initialize();
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

