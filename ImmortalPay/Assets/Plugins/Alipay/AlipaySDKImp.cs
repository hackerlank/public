using System;
using UnityEngine;
using System.Collections;
using System.Collections.Generic;
using System.Runtime.InteropServices;

namespace com.alipaysdk
{
	public abstract class AlipaySDKImpl
	{
		public abstract void Pay(string appScheme,string orderString);
	}
}