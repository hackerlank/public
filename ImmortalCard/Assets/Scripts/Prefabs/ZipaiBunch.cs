using UnityEngine;
using UnityEngine.UI;
using UnityEngine.EventSystems;
using System.Collections;
using Proto3;

public class ZipaiBunch : Bunch{
	public Text			Type;
	public Text			Score;

	public bool ShowType=false;

	override public bunch_t Value{
		get{
			return _bunch;
		}
		set{
			_bunch=value;
			//type and score
			if(ShowType){
				Type.text=type2str(value.Type);
				Score.text=PaohuziRule.CalcPoints(value).ToString();
				Type.gameObject.SetActive(true);
				Score.gameObject.SetActive(true);
			}
			//handle AAA & AAAA
			var bunch=new bunch_t(value);
			if(!ShowType){
				switch(bunch.Type){
				case pb_enum.PhzAaaadesk:
				case pb_enum.PhzAaaa:
				case pb_enum.PhzAaawei:
				case pb_enum.PhzAaachou:
					for(int i=1;i<bunch.Pawns.Count;++i)bunch.Pawns[i]=1000;
					break;
				default:
					break;
				}
			}
			//cards
			var ctrl=Main.Instance.gameController as GamePanel;
			foreach(var card in bunch.Pawns){
				Card.Create(ctrl.Rule.CardPrefab,card,Cards,delegate(Card obj) {
					obj.state=Card.State.ST_MELD;
				});
			}
		}
	}

	string type2str(pb_enum type){
		switch(type){
		case pb_enum.PhzAa:
			return "将";
		case pb_enum.PhzAbc:
			return "吃";
		case pb_enum.PhzAaawei:
		case pb_enum.PhzAaachou:
			return "偎";
		case pb_enum.PhzAaaa:
		case pb_enum.PhzAaaadesk:
		case pb_enum.PhzAaaastart:
			return "提";
		case pb_enum.PhzBbb:
			return "碰";
		case pb_enum.PhzBbbB:
		case pb_enum.PhzB4B3:
		case pb_enum.PhzBbbbdesk:
			return "跑";
		case pb_enum.PhzAaa:
			return "坎";
		default:
			return "";
		}
	}

	static public void PlaySound(pb_enum type){
		var sndName="";
		switch(type){
		case pb_enum.PhzAbc:
			sndName="m0_chi";
			break;
		case pb_enum.PhzAaawei:
		case pb_enum.PhzAaachou:
			sndName="m0_wei";
			break;
		case pb_enum.PhzAaaa:
		case pb_enum.PhzAaaadesk:
		case pb_enum.PhzAaaastart:
			sndName="m0_ti";
			break;
		case pb_enum.PhzBbb:
			sndName="m0_peng";
			break;
		case pb_enum.PhzBbbB:
		case pb_enum.PhzB4B3:
		case pb_enum.PhzBbbbdesk:
			sndName="m0_pso";
			break;
		default:
			break;
		}

		var snd=Audio.Instance.Get(sndName);
		if(null!=snd && !snd.isPlaying)
			snd.Play();
	}
}
