using UnityEngine;
using UnityEngine.UI;
using System.Collections;
using System.Collections.Generic;
using Proto3;

public class PaohuziSettleItem : SettleItem {
	public Text			Chunk,Multiple;
	public Transform	Bunches;

	override public play_t Value{
		set{
			base.Value=value;
			Chunk.text=value.Chunk.ToString();
			Multiple.text=value.Multiple.ToString();

			var bunches=new List<bunch_t>(value.Bunch);
			//hands
			var hands=new List<int>(value.Hands);
			bunches.AddRange(PaohuziRule.buildFrees(hands,bunches.Count));

			foreach(var bunch in bunches){
				Utils.Load<ZipaiBunch>(Bunches,delegate(Component obj) {
					var zi=obj as ZipaiBunch;
					zi.ShowType=true;
					zi.Value=bunch;
				});
			}
		}
	}
}
