using UnityEngine;
using UnityEngine.UI;
using System.Collections;
using Proto3;

public class PaohuziSettleItem : SettleItem {
	public Text			Chunk,Multiple;
	public Transform	Bunches;

	override public play_t Value{
		set{
			base.Value=value;
			Chunk.text=value.Chunk.ToString();
			Multiple.text=value.Multiple.ToString();

			foreach(var bunch in value.Bunch){
				Utils.Load<ZipaiBunch>(Bunches,delegate(Component obj) {
					var zi=obj as ZipaiBunch;
					zi.ShowType=true;
					zi.Value=bunch;
				});
			}
		}
	}
}
