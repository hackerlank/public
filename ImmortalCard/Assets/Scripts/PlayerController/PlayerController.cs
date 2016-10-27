using UnityEngine;
using System.Collections;
using Google.Protobuf;

public interface PlayerController {
	IEnumerator onMessage(Player player,IMessage msg);
}
