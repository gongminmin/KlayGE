using System.Collections.Generic;

namespace MtlEditor
{
	public class MaterialEntity
	{
		public uint ID { get; set; }

		public string Name { get; set; }

		public IList<MaterialEntity> Children
		{
			get
			{
				return children_;
			}
		}

		private readonly IList<MaterialEntity> children_ = new List<MaterialEntity>();
	}
}