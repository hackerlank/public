using UnityEngine;
using UnityEngine.UI;
using System.Collections;

public class TokenIcon : MonoBehaviour {
	public Text pile;
	public Transform token;

	int _pile=0;
	public int Pile{
		get{
			return _pile;
		}
		set{
			_pile=value;
			pile.text=value.ToString();
		}
	}

	public int Token{
		set{
			var gc=Main.Instance.gameController;
			if(gc!=null){
				var angle=0;
				if(null!=gc.Rule){
					var M=gc.Rule.MaxPlayer;
					if(value==0)
						angle=180;
					else if(value==1)
						angle=-90;
					else if(value==M-1)
						angle=90;
					else
						angle=0;
				}
				token.localEulerAngles=new Vector3(
					token.localEulerAngles.x,
					token.localEulerAngles.y,
					angle);
			}
		}
	}
}
