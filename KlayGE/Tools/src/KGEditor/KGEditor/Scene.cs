using System.Collections.ObjectModel;
using System.Diagnostics;
using System.Linq;

namespace KGEditor
{
	public class Scene
	{
		public Scene(MainWindow wnd)
		{
			wnd_ = wnd;

			name_ = "Scene";
			skybox_name_ = "";
			active_camera_id_ = 0;

			var category_list = new SceneEntityViewModel[3];

			var entity = new SceneEntity();
			entity.Id = 0;
			entity.Name = "Models";
			entity.Type = SceneEntityType.ET_Model;
			category_list[0] = new SceneEntityViewModel(wnd, entity);

			entity = new SceneEntity();
			entity.Id = 0;
			entity.Name = "Lights";
			entity.Type = SceneEntityType.ET_Light;
			category_list[1] = new SceneEntityViewModel(wnd, entity);

			entity = new SceneEntity();
			entity.Id = 0;
			entity.Name = "Cameras";
			entity.Type = SceneEntityType.ET_Camera;
			category_list[2] = new SceneEntityViewModel(wnd, entity);

			scene_entity_category_ = new ReadOnlyCollection<SceneEntityViewModel>(category_list);
		}

		public string Name
		{
			get
			{
				return name_;
			}
			set
			{
				if (name_ != value)
				{
					name_ = value;
					MainWindow.KGEditNativeCore.SceneName(name_);
				}
			}
		}
		public string SkyboxName
		{
			get
			{
				return skybox_name_;
			}
			set
			{
				if (skybox_name_ != value)
				{
					skybox_name_ = value;
					MainWindow.KGEditNativeCore.SkyboxName(skybox_name_);
				}
			}
		}

		public ReadOnlyCollection<SceneEntityViewModel> SceneRoot
		{
			get
			{
				return scene_entity_category_;
			}
		}

		public uint ActiveCameraId
		{
			get
			{
				return active_camera_id_;
			}
			set
			{
				if (active_camera_id_ != value)
				{
					active_camera_id_ = value;
					MainWindow.KGEditNativeCore.ActiveCameraId(active_camera_id_);
				}
			}
		}

		public SceneEntityViewModel FindSceneEntity(uint entity_id)
		{
			foreach (var category in scene_entity_category_)
			{
				foreach (var entity in category.Children)
				{
					if (entity.Entity.Id == entity_id)
					{
						return entity;
					}
				}
			}

			return null;
		}

		public void AddEntity(uint entity_id, SceneEntityType type)
		{
			Debug.Assert(entity_id > 0);

			SceneEntity entity;
			switch (type)
			{
				case SceneEntityType.ET_Model:
					entity = new SceneEntity();
					break;

				case SceneEntityType.ET_Light:
					entity = new SceneEntityLight();
					break;

				case SceneEntityType.ET_Camera:
				default:
					entity = new SceneEntityCamera();
					break;
			}

			entity.Id = entity_id;
			entity.Type = type;
			scene_entity_category_[(int)type].Children.Add(new SceneEntityViewModel(wnd_, entity));
			scene_entity_category_[(int)type].Children.Last().SelectedInternal(true);

			entity.RetrieveProperties();
		}

		public void RemoveEntity(uint entity_id)
		{
			Debug.Assert(entity_id > 0);

			foreach (var category in scene_entity_category_)
			{
				foreach (var entity in category.Children)
				{
					if (entity.Entity.Id == entity_id)
					{
						category.Children.Remove(entity);
						break;
					}
				}
			}

			MainWindow.KGEditNativeCore.RemoveEntity(entity_id);
		}

		public virtual void RetrieveProperties()
		{
			var core = MainWindow.KGEditNativeCore;

			name_ = core.SceneName();
			skybox_name_ = core.SkyboxName();
			active_camera_id_ = core.ActiveCameraId();
		}

		private readonly MainWindow wnd_;
		private string name_;
		private string skybox_name_;
		private uint active_camera_id_;
		private readonly ReadOnlyCollection<SceneEntityViewModel> scene_entity_category_;
	}
}