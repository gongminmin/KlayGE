using System.Collections.Generic;

namespace MtlEditor
{
	public class MeshEntity
	{
		public uint ID { get; set; }

		public string Name { get; set; }

		public IList<MeshEntity> Children
		{
			get
			{
				return children_;
			}
		}

		private readonly IList<MeshEntity> children_ = new List<MeshEntity>();
	}
}