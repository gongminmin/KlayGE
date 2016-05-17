using System;
using System.Collections.Generic;

namespace KGEditor
{
	public class SceneEntity
	{
		public uint ID { get; set; }

		public string Name { get; set; }
		public KlayGE.KGEditorCoreWrapper.EntityType Type { get; set; }

		public IList<SceneEntity> Children
		{
			get
			{
				return children_;
			}
		}

		private readonly IList<SceneEntity> children_ = new List<SceneEntity>();
	}
}