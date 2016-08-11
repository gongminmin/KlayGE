using System;
using System.Collections.ObjectModel;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Forms;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using System.ComponentModel;
using System.Windows.Controls.Ribbon;
using Xceed.Wpf.Toolkit.PropertyGrid.Attributes;
using KlayGE;

namespace KGEditor
{
	public enum ControlModeEnum
	{
		CM_EntitySelection = KGEditorCoreWrapper.ControlMode.CM_EntitySelection,
		CM_EntityTranslation = KGEditorCoreWrapper.ControlMode.CM_EntityTranslation,
		CM_EntityRotation = KGEditorCoreWrapper.ControlMode.CM_EntityRotation,
		CM_EntityScaling = KGEditorCoreWrapper.ControlMode.CM_EntityScaling,
	}

	/// <summary>
	/// Interaction logic for MainWindow.xaml
	/// </summary>
	public partial class MainWindow : RibbonWindow
	{
		enum SystemProperties
		{
			SP_SystemSceneName = 0,
			SP_SystemActiveCamera,
			SP_SystemSkyBox,
			SP_SystemDisplaySSVO,
			SP_SystemDisplayHDR,
			SP_SystemDisplayAA,
			SP_SystemDisplayGamma,
			SP_SystemDisplayColorGrading,

			Num_SystemProperties
		}

		enum EntityProperties
		{
			EP_EntityName,
			EP_HideEntity,
			EP_EntityTranslationX,
			EP_EntityTranslationY,
			EP_EntityTranslationZ,
			EP_EntityRotationQuatX,
			EP_EntityRotationQuatY,
			EP_EntityRotationQuatZ,
			EP_EntityRotationQuatW,
			EP_EntityRotationAngleYaw,
			EP_EntityRotationAnglePitch,
			EP_EntityRotationAngleRoll,
			EP_EntityScalingX,
			EP_EntityScalingY,
			EP_EntityScalingZ,

			Num_EntityProperties
		}

		enum LightProperties
		{
			LP_Type = EntityProperties.Num_EntityProperties,
			LP_Enabled,
			LP_AttributeNoShadow,
			LP_AttributeNoDiffuse,
			LP_AttributeNoSpecular,
			LP_AttributeIndirect,
			LP_Color,
			LP_Multiplier,
			LP_FalloffConstant,
			LP_FalloffLinear,
			LP_FalloffQuadratic,
			LP_InnerAngle,
			LP_OuterAngle,
			LP_ProjectiveTex,

			Num_LightProperties
		};

		enum CameraProperties
		{
			CP_LookAtX = EntityProperties.Num_EntityProperties,
			CP_LookAtY,
			CP_LookAtZ,
			CP_UpVecX,
			CP_UpVecY,
			CP_UpVecZ,
			CP_FoV,
			CP_Aspect,
			CP_NearPlane,
			CP_FarPlane,

			Num_CameraProperties
		}

		[CategoryOrder("System", 0)]
		public class SystemPropertyType
		{
			[Category("System")]
			[DisplayName("System")]
			[PropertyOrder((int)SystemProperties.SP_SystemSceneName)]
			public string SceneName { get; set; }

			[Category("System")]
			[DisplayName("Active Camera")]
			[ItemsSource(typeof(CameraItemsSource))]
			[PropertyOrder((int)SystemProperties.SP_SystemActiveCamera)]
			public string ActiveCamera { get; set; }

			[Category("System")]
			[DisplayName("Skybox")]
			[PropertyOrder((int)SystemProperties.SP_SystemSkyBox)]
			[Editor(typeof(OpenTexUserControlEditor), typeof(OpenTexUserControlEditor))]
			public string SkyBox { get; set; }

			[Category("System")]
			[DisplayName("SSVO")]
			[PropertyOrder((int)SystemProperties.SP_SystemDisplaySSVO)]
			public bool SSVO { get; set; }

			[Category("System")]
			[DisplayName("HDR")]
			[PropertyOrder((int)SystemProperties.SP_SystemDisplayHDR)]
			public bool HDR { get; set; }

			[Category("System")]
			[DisplayName("AA")]
			[PropertyOrder((int)SystemProperties.SP_SystemDisplayAA)]
			public bool AA { get; set; }

			[Category("System")]
			[DisplayName("Gamma")]
			[PropertyOrder((int)SystemProperties.SP_SystemDisplayGamma)]
			public bool Gamma { get; set; }

			[Category("System")]
			[DisplayName("Color Grading")]
			[PropertyOrder((int)SystemProperties.SP_SystemDisplayColorGrading)]
			public bool ColorGrading { get; set; }
		}

		[CategoryOrder("Properties", 0)]
		[CategoryOrder("Translation", 1)]
		[CategoryOrder("Rotation", 2)]
		[CategoryOrder("Scaling", 3)]
		public class EntityPropertyType
		{
			[Category("Properties")]
			[DisplayName("Name")]
			[PropertyOrder((int)EntityProperties.EP_EntityName)]
			public string Name { get; set; }

			[Category("Properties")]
			[DisplayName("Hide")]
			[PropertyOrder((int)EntityProperties.EP_HideEntity)]
			public bool Hide { get; set; }

			[Category("Translation")]
			[DisplayName("X")]
			[PropertyOrder((int)EntityProperties.EP_EntityTranslationX)]
			public float TranslationX { get; set; }

			[Category("Translation")]
			[DisplayName("Y")]
			[PropertyOrder((int)EntityProperties.EP_EntityTranslationY)]
			public float TranslationY { get; set; }

			[Category("Translation")]
			[DisplayName("Z")]
			[PropertyOrder((int)EntityProperties.EP_EntityTranslationZ)]
			public float TranslationZ { get; set; }

			[Category("Rotation")]
			[DisplayName("Quaternion X")]
			[PropertyOrder((int)EntityProperties.EP_EntityRotationQuatX)]
			public float RotationQuatX { get; set; }

			[Category("Rotation")]
			[DisplayName("Quaternion Y")]
			[PropertyOrder((int)EntityProperties.EP_EntityRotationQuatY)]
			public float RotationQuatY { get; set; }

			[Category("Rotation")]
			[DisplayName("Quaternion Z")]
			[PropertyOrder((int)EntityProperties.EP_EntityRotationQuatZ)]
			public float RotationQuatZ { get; set; }

			[Category("Rotation")]
			[DisplayName("Quaternion W")]
			[PropertyOrder((int)EntityProperties.EP_EntityRotationQuatW)]
			public float RotationQuatW { get; set; }

			[Category("Rotation")]
			[DisplayName("Yaw")]
			[PropertyOrder((int)EntityProperties.EP_EntityRotationAngleYaw)]
			public float RotationYaw { get; set; }

			[Category("Rotation")]
			[DisplayName("Pitch")]
			[PropertyOrder((int)EntityProperties.EP_EntityRotationAnglePitch)]
			public float RotationPitch { get; set; }

			[Category("Rotation")]
			[DisplayName("Roll")]
			[PropertyOrder((int)EntityProperties.EP_EntityRotationAngleRoll)]
			public float RotationRoll { get; set; }

			[Category("Scaling")]
			[DisplayName("X")]
			[PropertyOrder((int)EntityProperties.EP_EntityScalingX)]
			public float ScalingX { get; set; }

			[Category("Scaling")]
			[DisplayName("Y")]
			[PropertyOrder((int)EntityProperties.EP_EntityScalingY)]
			public float ScalingY { get; set; }

			[Category("Scaling")]
			[DisplayName("Z")]
			[PropertyOrder((int)EntityProperties.EP_EntityScalingZ)]
			public float ScalingZ { get; set; }
		}

		[CategoryOrder("Properties", 0)]
		[CategoryOrder("Translation", 1)]
		[CategoryOrder("Rotation", 2)]
		[CategoryOrder("Scaling", 3)]
		[CategoryOrder("Light", 4)]
		public class LightPropertyType : EntityPropertyType
		{
			[Category("Light")]
			[DisplayName("Type")]
			[ItemsSource(typeof(LightTypeItemsSource))]
			[PropertyOrder((int)LightProperties.LP_Type)]
			public string Type { get; set; }

			[Category("Light")]
			[DisplayName("Enabled")]
			[PropertyOrder((int)LightProperties.LP_Enabled)]
			public bool Enabled { get; set; }

			[Category("Light")]
			[DisplayName("No Shadow")]
			[PropertyOrder((int)LightProperties.LP_AttributeNoShadow)]
			public bool NoShadow { get; set; }

			[Category("Light")]
			[DisplayName("No Diffuse")]
			[PropertyOrder((int)LightProperties.LP_AttributeNoDiffuse)]
			public bool NoDiffuse { get; set; }

			[Category("Light")]
			[DisplayName("No Specular")]
			[PropertyOrder((int)LightProperties.LP_AttributeNoSpecular)]
			public bool NoSpecular { get; set; }

			[Category("Light")]
			[DisplayName("Indirect Lighting")]
			[PropertyOrder((int)LightProperties.LP_AttributeIndirect)]
			public bool Indirect { get; set; }

			[Category("Light")]
			[DisplayName("Color")]
			[PropertyOrder((int)LightProperties.LP_Color)]
			public Color Color { get; set; }

			[Category("Light")]
			[DisplayName("Multiplier")]
			[PropertyOrder((int)LightProperties.LP_Multiplier)]
			[Editor(typeof(MultiplierSliderUserControlEditor), typeof(SliderUserControlEditor))]
			public float Multiplier { get; set; }

			[Category("Light")]
			[DisplayName("Falloff Constant")]
			[PropertyOrder((int)LightProperties.LP_FalloffConstant)]
			[Editor(typeof(SliderUserControlEditor), typeof(SliderUserControlEditor))]
			public float FalloffConstant { get; set; }

			[Category("Light")]
			[DisplayName("Falloff Linear")]
			[PropertyOrder((int)LightProperties.LP_FalloffLinear)]
			[Editor(typeof(SliderUserControlEditor), typeof(SliderUserControlEditor))]
			public float FalloffLinear { get; set; }

			[Category("Light")]
			[DisplayName("Falloff Quadratic")]
			[PropertyOrder((int)LightProperties.LP_FalloffQuadratic)]
			[Editor(typeof(SliderUserControlEditor), typeof(SliderUserControlEditor))]
			public float FalloffQuadratic { get; set; }

			[Category("Light")]
			[DisplayName("Inner Angle")]
			[PropertyOrder((int)LightProperties.LP_InnerAngle)]
			[Editor(typeof(InnerAngleSliderUserControlEditor), typeof(SliderUserControlEditor))]
			public float InnerAngle { get; set; }

			[Category("Light")]
			[DisplayName("Outer Angle")]
			[PropertyOrder((int)LightProperties.LP_OuterAngle)]
			[Editor(typeof(OuterAngleSliderUserControlEditor), typeof(SliderUserControlEditor))]
			public float OuterAngle { get; set; }

			[Category("Light")]
			[DisplayName("Projective Texture")]
			[PropertyOrder((int)LightProperties.LP_ProjectiveTex)]
			[Editor(typeof(OpenTexUserControlEditor), typeof(OpenTexUserControlEditor))]
			public string ProjectiveTex { get; set; }
		}

		[CategoryOrder("Properties", 0)]
		[CategoryOrder("Translation", 1)]
		[CategoryOrder("Rotation", 2)]
		[CategoryOrder("Scaling", 3)]
		[CategoryOrder("Camera", 4)]
		public class CameraPropertyType : EntityPropertyType
		{
			[Category("Camera")]
			[DisplayName("Look At X")]
			[PropertyOrder((int)CameraProperties.CP_LookAtX)]
			public float LookAtX { get; set; }

			[Category("Camera")]
			[DisplayName("Look At Y")]
			[PropertyOrder((int)CameraProperties.CP_LookAtY)]
			public float LookAtY { get; set; }

			[Category("Camera")]
			[DisplayName("Look At Z")]
			[PropertyOrder((int)CameraProperties.CP_LookAtZ)]
			public float LookAtZ { get; set; }

			[Category("Camera")]
			[DisplayName("Up Vector X")]
			[PropertyOrder((int)CameraProperties.CP_UpVecX)]
			public float UpVecX { get; set; }

			[Category("Camera")]
			[DisplayName("Up Vector Y")]
			[PropertyOrder((int)CameraProperties.CP_UpVecY)]
			public float UpVecY { get; set; }

			[Category("Camera")]
			[DisplayName("Up Vector Z")]
			[PropertyOrder((int)CameraProperties.CP_UpVecZ)]
			public float UpVecZ { get; set; }

			[Category("Camera")]
			[DisplayName("FoV")]
			[PropertyOrder((int)CameraProperties.CP_FoV)]
			public float FoV { get; set; }

			[Category("Camera")]
			[DisplayName("Aspect")]
			[PropertyOrder((int)CameraProperties.CP_Aspect)]
			public float Aspect { get; set; }

			[Category("Camera")]
			[DisplayName("Near Plane")]
			[PropertyOrder((int)CameraProperties.CP_NearPlane)]
			public float NearPlane { get; set; }

			[Category("Camera")]
			[DisplayName("Far Plane")]
			[PropertyOrder((int)CameraProperties.CP_FarPlane)]
			public float FarPlane { get; set; }
		}

		public MainWindow()
		{
			InitializeComponent();

			DataContext = this;

			var category_list = new SceneEntityViewModel[3];

			var entity = new SceneEntity();
			entity.ID = 0;
			entity.Name = "Models";
			entity.Type = KGEditorCoreWrapper.EntityType.ET_Model;
			category_list[0] = new SceneEntityViewModel(this, entity);

			entity = new SceneEntity();
			entity.ID = 0;
			entity.Name = "Lights";
			entity.Type = KGEditorCoreWrapper.EntityType.ET_Light;
			category_list[1] = new SceneEntityViewModel(this, entity);

			entity = new SceneEntity();
			entity.ID = 0;
			entity.Name = "Cameras";
			entity.Type = KGEditorCoreWrapper.EntityType.ET_Camera;
			category_list[2] = new SceneEntityViewModel(this, entity);

			scene_entity_category_ = new ReadOnlyCollection<SceneEntityViewModel>(category_list);

			CameraItemsSource.items.Clear();
			CameraItemsSource.items.Add("System");

			system_properties_obj_.ActiveCamera = CameraItemsSource.items[0].Value as string;
			system_properties_obj_.SSVO = true;
			system_properties_obj_.HDR = true;
			system_properties_obj_.AA = true;
			system_properties_obj_.Gamma = true;
			system_properties_obj_.ColorGrading = true;
			properties.SelectedObject = system_properties_obj_;

			save.IsEnabled = false;
			save_as.IsEnabled = false;
			undo.IsEnabled = false;
			redo.IsEnabled = false;

			Uri iconUri = new Uri("pack://application:,,,/Images/klayge_logo.ico", UriKind.RelativeOrAbsolute);
			this.Icon = BitmapFrame.Create(iconUri);
		}

		void MainWindowLoaded(object sender, RoutedEventArgs e)
		{
			IntPtr wnd = editor_wnd.Handle;
			core_ = new KGEditorCoreWrapper(wnd);

			core_.UpdatePropertyCallback(this.UpdatePropList);
			core_.UpdateSelectEntityCallback(this.UpdateSelectEntity);
			core_.UpdateAddEntityCallback(this.UpdateAddEntity);
			core_.UpdateRemoveEntityCallback(this.UpdateRemoveEntity);
			core_.AddModelCallback(this.AddModel);
			core_.AddLightCallback(this.AddLight);
			core_.AddCameraCallback(this.AddCamera);

			CompositionTarget.Rendering += this.MainWindowIdle;
		}

		void MainWindowUnloaded(object sender, RoutedEventArgs e)
		{
			CompositionTarget.Rendering -= this.MainWindowIdle;
			editor_frame.Dispose();
			core_.Dispose();
			core_ = null;
		}

		private void MainWindowIdle(object sender, EventArgs e)
		{
			core_.Refresh();
		}

		private void EditorWindowSizeChanged(object sender, SizeChangedEventArgs e)
		{
			editor_frame.Width = editor_bg.ActualWidth;
			editor_frame.Height = editor_bg.ActualHeight;

			core_.Resize((uint)editor_frame.Width, (uint)editor_frame.Height);
		}

		private void LoadScene(string file_name)
		{
			string ext_name = System.IO.Path.GetExtension(file_name).ToLower();
			if (ext_name != ".kges")
			{
				return;
			}

			core_.LoadScene(file_name);
			this.FileNameChanged(file_name);

			this.ClearHistroy();
			this.UpdateHistroy();

			save.IsEnabled = true;
			save_as.IsEnabled = true;

			this.UpdateEntityProperties(0);
		}

		private void OpenClick(object sender, RoutedEventArgs e)
		{
			Microsoft.Win32.OpenFileDialog dlg = new Microsoft.Win32.OpenFileDialog();

			dlg.DefaultExt = ".kges";
			dlg.Filter = "KGES Files (*.kges)|*.kges|All Files|*.*";
			dlg.CheckPathExists = true;
			dlg.CheckFileExists = true;
			if (true == dlg.ShowDialog())
			{
				this.LoadScene(dlg.FileName);
			}
		}

		private void SaveClick(object sender, RoutedEventArgs e)
		{
			if ("" == opened_file_)
			{
				this.SaveAsClick(sender, e);
			}
			else
			{
				core_.SaveScene(opened_file_);
			}
		}

		private void SaveAsClick(object sender, RoutedEventArgs e)
		{
			Microsoft.Win32.SaveFileDialog dlg = new Microsoft.Win32.SaveFileDialog();

			dlg.DefaultExt = ".kges";
			dlg.Filter = "KGES Files (*.kges)|*.kges|All Files|*.*";
			dlg.OverwritePrompt = true;
			if (true == dlg.ShowDialog())
			{
				core_.SaveScene(dlg.FileName);
				this.FileNameChanged(dlg.FileName);
			}
		}

		private void AddModelClick(object sender, RoutedEventArgs e)
		{
			Microsoft.Win32.OpenFileDialog dlg = new Microsoft.Win32.OpenFileDialog();

			dlg.DefaultExt = ".meshml";
			dlg.Filter = "All Model Files (*.meshml, *.model_bin)|*.meshml;*.model_bin|MeshML Files (*.meshml)|*.meshml|model_bin Files (*.model_bin)|*.model_bin|All Files|*.*";
			dlg.CheckPathExists = true;
			dlg.CheckFileExists = true;
			if (true == dlg.ShowDialog())
			{
				this.AddModel(dlg.FileName);

				save.IsEnabled = true;
				save_as.IsEnabled = true;
			}
		}

		private void AddAmbientLightClick(object sender, RoutedEventArgs e)
		{
			this.AddLight(KGEditorCoreWrapper.LightType.LT_Ambient);

			save.IsEnabled = true;
			save_as.IsEnabled = true;
		}

		private void AddDirectionalLightClick(object sender, RoutedEventArgs e)
		{
			this.AddLight(KGEditorCoreWrapper.LightType.LT_Directional);

			save.IsEnabled = true;
			save_as.IsEnabled = true;
		}

		private void AddPointLightClick(object sender, RoutedEventArgs e)
		{
			this.AddLight(KGEditorCoreWrapper.LightType.LT_Point);

			save.IsEnabled = true;
			save_as.IsEnabled = true;
		}

		private void AddSpotLightClick(object sender, RoutedEventArgs e)
		{
			this.AddLight(KGEditorCoreWrapper.LightType.LT_Spot);

			save.IsEnabled = true;
			save_as.IsEnabled = true;
		}

		private void AddCameraClick(object sender, RoutedEventArgs e)
		{
			this.AddCamera();

			save.IsEnabled = true;
			save_as.IsEnabled = true;
		}

		private void RemoveEntityClick(object sender, RoutedEventArgs e)
		{
			this.RemoveEntity(selected_entity_id_);
		}

		public ControlModeEnum ControlModeProperty
		{
			get
			{
				return (ControlModeEnum)control_mode_;
			}
			set
			{
				this.ControlMode((KGEditorCoreWrapper.ControlMode)value);
			}
		}

		private void AddModel(string file_name)
		{
			string ext_name = System.IO.Path.GetExtension(file_name);
			if ((ext_name != ".meshml") && (ext_name != ".model_bin"))
			{
				return;
			}

			this.ExecuteCommand(new KGEditorCommandAddModel(core_, file_name));
		}

		private void AddLight(KGEditorCoreWrapper.LightType type)
		{
			string type_str;
			switch (type)
			{
				case KGEditorCoreWrapper.LightType.LT_Ambient:
					type_str = "Ambient";
					break;

				case KGEditorCoreWrapper.LightType.LT_Directional:
					type_str = "Directional";
					break;

				case KGEditorCoreWrapper.LightType.LT_Point:
					type_str = "Point";
					break;

				case KGEditorCoreWrapper.LightType.LT_Spot:
				default:
					type_str = "Spot";
					break;
			}

			string light_name;
			int index = 0;
			for (;;)
			{
				light_name = type_str + index.ToString();
				bool found = false;
				foreach (var light in scene_entity_category_[1].Children)
				{
					if (core_.EntityName(light.Entity.ID) == light_name)
					{
						found = true;
						break;
					}
				}

				if (!found)
				{
					break;
				}

				++ index;
			}

			this.ExecuteCommand(new KGEditorCommandAddLight(core_, type, light_name));
		}

		private void AddCamera()
		{
			string camera_name;
			int index = 0;
			for (;;)
			{
				camera_name = "Camera" + index.ToString();
				bool found = false;
				foreach (var camera in scene_entity_category_[2].Children)
				{
					if (core_.EntityName(camera.Entity.ID) == camera_name)
					{
						found = true;
						break;
					}
				}

				if (!found)
				{
					break;
				}

				++ index;
			}

			this.ExecuteCommand(new KGEditorCommandAddCamera(core_, camera_name));
		}

		private void UndoClick(object sender, RoutedEventArgs e)
		{
			this.Undo();

			selected_entity_id_ = core_.SelectedEntity();
			this.UpdateEntityProperties(selected_entity_id_);
			this.UpdateHistroy();
		}

		private void RedoClick(object sender, RoutedEventArgs e)
		{
			this.Redo();

			selected_entity_id_ = core_.SelectedEntity();
			this.UpdateEntityProperties(selected_entity_id_);
			this.UpdateHistroy();
		}

		private void EditorMouseDown(object sender, System.Windows.Forms.MouseEventArgs e)
		{
			uint buttons = 0;
			if (MouseButtons.Left == e.Button)
			{
				buttons |= 1;
			}
			if (MouseButtons.Right == e.Button)
			{
				buttons |= 2;
			}
			if (MouseButtons.Middle == e.Button)
			{
				buttons |= 4;
			}
			core_.MouseDown(e.X, e.Y, buttons);
		}

		private void EditorMouseUp(object sender, System.Windows.Forms.MouseEventArgs e)
		{
			uint buttons = 0;
			if (MouseButtons.Left == e.Button)
			{
				buttons |= 1;
			}
			if (MouseButtons.Right == e.Button)
			{
				buttons |= 2;
			}
			if (MouseButtons.Middle == e.Button)
			{
				buttons |= 4;
			}
			core_.MouseUp(e.X, e.Y, buttons);
		}

		private void EditorMouseMove(object sender, System.Windows.Forms.MouseEventArgs e)
		{
			uint buttons = 0;
			if (MouseButtons.Left == e.Button)
			{
				buttons |= 1;
			}
			if (MouseButtons.Right == e.Button)
			{
				buttons |= 2;
			}
			if (MouseButtons.Middle == e.Button)
			{
				buttons |= 4;
			}
			core_.MouseMove(e.X, e.Y, buttons);
		}

		private void EditorKeyUp(object sender, System.Windows.Input.KeyEventArgs e)
		{
			switch (e.Key)
			{
				case Key.Delete:
					this.RemoveEntity(selected_entity_id_);
					break;

				case Key.Q:
					// TODO: Figure out how to use data binding on this.
					selection.IsChecked = true;
					break;

				case Key.W:
					translation.IsChecked = true;
					break;

				case Key.E:
					rotation.IsChecked = true;
					break;

				case Key.R:
					scaling.IsChecked = true;
					break;

				default:
					break;
			}
		}

		public void SelectEntity(uint entity_id)
		{
			this.ExecuteCommand(new KGEditorCommandSelectEntity(core_, entity_id));
			this.UpdateEntityProperties(entity_id);
		}

		public void RemoveEntity(uint entity_id)
		{
			if (entity_id > 0)
			{
				foreach (var category in scene_entity_category_)
				{
					foreach (var entity in category.Children)
					{
						if (entity.Entity.ID == entity_id)
						{
							category.Children.Remove(entity);
							break;
						}
					}
				}

				this.ExecuteCommand(new KGEditorCommandRemoveEntity(core_, entity_id));
			}
		}

		private void ControlMode(KGEditorCoreWrapper.ControlMode mode)
		{
			control_mode_ = mode;
			this.ExecuteCommand(new KGEditorCommandSetControlMode(core_, mode));
		}

		private void UpdateEntityProperties(uint entity_id)
		{
			selected_entity_id_ = entity_id;

			properties.SelectedObject = null;

			if (0 == selected_entity_id_)
			{
				properties.SelectedObject = system_properties_obj_;
			}
			else
			{
				switch (core_.GetEntityType(selected_entity_id_))
				{
					case KGEditorCoreWrapper.EntityType.ET_Model:
						properties.SelectedObject = entity_properties_obj_;
						break;

					case KGEditorCoreWrapper.EntityType.ET_Light:
						properties.SelectedObject = light_properties_obj_;
						break;

					case KGEditorCoreWrapper.EntityType.ET_Camera:
					default:
						properties.SelectedObject = camera_properties_obj_;
						break;
				}
			}
		}

		private void UpdatePropList()
		{
			int active_prop_list = -1;

			if (core_ != null)
			{
				selected_entity_id_ = core_.SelectedEntity();
				if (selected_entity_id_ > 0)
				{
					float[] translation = core_.EntityTranslation(selected_entity_id_);
					float[] quat = core_.EntityRotation(selected_entity_id_);
					float[] yaw_pitch_roll = this.QuatToYawPitchRoll(quat);
					float[] scaling = core_.EntityScaling(selected_entity_id_);

					active_prop_list = (int)core_.GetEntityType(selected_entity_id_);

					EntityPropertyType entity_common_props;
					switch ((KGEditorCoreWrapper.EntityType)active_prop_list)
					{
						case KGEditorCoreWrapper.EntityType.ET_Model:
							entity_common_props = entity_properties_obj_;
							break;

						case KGEditorCoreWrapper.EntityType.ET_Light:
							entity_common_props = light_properties_obj_;
							break;

						case KGEditorCoreWrapper.EntityType.ET_Camera:
						default:
							entity_common_props = camera_properties_obj_;
							break;
					}

					entity_common_props.Name = core_.EntityName(selected_entity_id_);
					entity_common_props.Hide = core_.HideEntity(selected_entity_id_);
					entity_common_props.TranslationX = translation[0];
					entity_common_props.TranslationY = translation[1];
					entity_common_props.TranslationZ = translation[2];
					entity_common_props.RotationQuatX = quat[0];
					entity_common_props.RotationQuatY = quat[1];
					entity_common_props.RotationQuatZ = quat[2];
					entity_common_props.RotationQuatW = quat[3];
					entity_common_props.RotationYaw = yaw_pitch_roll[0];
					entity_common_props.RotationPitch = yaw_pitch_roll[1];
					entity_common_props.RotationRoll = yaw_pitch_roll[2];
					entity_common_props.ScalingX = scaling[0];
					entity_common_props.ScalingY = scaling[1];
					entity_common_props.ScalingZ = scaling[2];

					switch ((KGEditorCoreWrapper.EntityType)active_prop_list)
					{
						case KGEditorCoreWrapper.EntityType.ET_Model:
							break;

						case KGEditorCoreWrapper.EntityType.ET_Light:
							{
								light_properties_obj_.Type
									= LightTypeItemsSource.items[(int)core_.GetLightType(selected_entity_id_)].Value as string;
								light_properties_obj_.Enabled = core_.LightEnabled(selected_entity_id_);
								int light_attrib = core_.LightAttrib(selected_entity_id_);
								light_properties_obj_.NoShadow
									= (light_attrib & (int)KGEditorCoreWrapper.LightSrcAttrib.LSA_NoShadow) != 0;
								light_properties_obj_.NoDiffuse
									= (light_attrib & (int)KGEditorCoreWrapper.LightSrcAttrib.LSA_NoDiffuse) != 0;
								light_properties_obj_.NoSpecular
									= (light_attrib & (int)KGEditorCoreWrapper.LightSrcAttrib.LSA_NoSpecular) != 0;
								light_properties_obj_.Indirect
									= (light_attrib & (int)KGEditorCoreWrapper.LightSrcAttrib.LSA_IndirectLighting) != 0;
								float[] light_color = core_.LightColor(selected_entity_id_);
								light_properties_obj_.Multiplier = this.FloatPtrToMultipiler(light_color);
								light_properties_obj_.Color = this.FloatPtrToLDRColor(light_color, light_properties_obj_.Multiplier);
								float[] light_falloff = core_.LightFalloff(selected_entity_id_);
								light_properties_obj_.FalloffConstant = light_falloff[0];
								light_properties_obj_.FalloffLinear = light_falloff[1];
								light_properties_obj_.FalloffQuadratic = light_falloff[2];
								light_properties_obj_.InnerAngle = (float)(core_.LightInnerAngle(selected_entity_id_) * (180 / Math.PI));
								light_properties_obj_.OuterAngle = (float)(core_.LightOuterAngle(selected_entity_id_) * (180 / Math.PI));
								light_properties_obj_.ProjectiveTex = core_.LightProjectiveTex(selected_entity_id_);
							}
							break;

						case KGEditorCoreWrapper.EntityType.ET_Camera:
						default:
							{
								float[] look_at = core_.CameraLookAt(selected_entity_id_);
								float[] up_vec = core_.CameraUpVec(selected_entity_id_);
								camera_properties_obj_.LookAtX = look_at[0];
								camera_properties_obj_.LookAtY = look_at[1];
								camera_properties_obj_.LookAtZ = look_at[2];
								camera_properties_obj_.UpVecX = up_vec[0];
								camera_properties_obj_.UpVecY = up_vec[1];
								camera_properties_obj_.UpVecZ = up_vec[2];
							}
							camera_properties_obj_.FoV = core_.CameraFoV(selected_entity_id_);
							camera_properties_obj_.Aspect = core_.CameraAspect(selected_entity_id_);
							camera_properties_obj_.NearPlane = core_.CameraNearPlane(selected_entity_id_);
							camera_properties_obj_.FarPlane = core_.CameraFarPlane(selected_entity_id_);
							break;
					}
				}
				else
				{
					system_properties_obj_.SceneName = core_.SceneName();
					system_properties_obj_.SkyBox = core_.SkyboxName();
				}
			}

			// TODO: Why it's very slow
			properties.SelectedObject = null;

			switch ((KGEditorCoreWrapper.EntityType)active_prop_list)
			{
				case KGEditorCoreWrapper.EntityType.ET_Model:
					properties.SelectedObject = entity_properties_obj_;
					break;

				case KGEditorCoreWrapper.EntityType.ET_Light:
					properties.SelectedObject = light_properties_obj_;
					break;

				case KGEditorCoreWrapper.EntityType.ET_Camera:
					properties.SelectedObject = camera_properties_obj_;
					break;

				default:
					properties.SelectedObject = system_properties_obj_;

					CameraItemsSource.items.Clear();
					CameraItemsSource.items.Add("System");

					foreach (var camera in scene_entity_category_[2].Children)
					{
						CameraItemsSource.items.Add(camera.Entity.Name);
					}
					break;
			}
		}

		private void UpdateSelectEntity(uint entity_id)
		{
			foreach (var category in scene_entity_category_)
			{
				foreach (var entity in category.Children)
				{
					entity.IsSelected = false;
				}
			}

			if (entity_id > 0)
			{
				foreach (var category in scene_entity_category_)
				{
					foreach (var entity in category.Children)
					{
						if (entity.Entity.ID == entity_id)
						{
							entity.IsSelected = true;
						}
					}
				}
			}
			else
			{
				this.SelectEntity(entity_id);
			}
		}

		private void UpdateAddEntity(uint entity_id)
		{
			if (entity_id > 0)
			{
				var entity = new SceneEntity();
				entity.ID = entity_id;
				entity.Name = core_.EntityName(entity_id);
				entity.Type = core_.GetEntityType(entity_id);
				int type = (int)core_.GetEntityType(entity_id);
				scene_entity_category_[type].Children.Add(new SceneEntityViewModel(this, entity));
				scene_entity_category_[type].Children.Last().SelectedInternal(true);
			}
		}

		private void UpdateRemoveEntity(uint entity_id)
		{
			if (entity_id > 0)
			{
				int type = (int)core_.GetEntityType(entity_id);
				foreach (var entity in scene_entity_category_[type].Children)
				{
					if (entity.Entity.ID == entity_id)
					{
						scene_entity_category_[type].Children.Remove(entity);
						break;
					}
				}
			}
		}

		private void PropertyGridValueChanged(object sender, Xceed.Wpf.Toolkit.PropertyGrid.PropertyValueChangedEventArgs e)
		{
			Xceed.Wpf.Toolkit.PropertyGrid.PropertyItem item = e.OriginalSource as Xceed.Wpf.Toolkit.PropertyGrid.PropertyItem;
			if (properties.SelectedObject == system_properties_obj_)
			{
				switch ((SystemProperties)item.PropertyOrder)
				{
					case SystemProperties.SP_SystemSceneName:
						if (core_.SceneName() != system_properties_obj_.SceneName)
						{
							this.ExecuteCommand(new KGEditorCommandSetSceneName(core_, system_properties_obj_.SceneName));
						}
						break;

					case SystemProperties.SP_SystemActiveCamera:
						{
							foreach (var camera in scene_entity_category_[2].Children)
							{
								if (system_properties_obj_.ActiveCamera == core_.EntityName(camera.Entity.ID))
								{
									if (core_.ActiveCamera() != camera.Entity.ID)
									{
										this.ExecuteCommand(new KGEditorCommandSetActiveCamera(core_, camera.Entity.ID));
									}
									break;
								}
							}
						}
						break;

					case SystemProperties.SP_SystemSkyBox:
						{
							string sky_box_name = RelativePath(system_properties_obj_.SkyBox);
							if (core_.SkyboxName() != sky_box_name)
							{
								this.ExecuteCommand(new KGEditorCommandSetSkyboxName(core_, sky_box_name));
							}
						}
						break;

					case SystemProperties.SP_SystemDisplaySSVO:
						core_.DisplaySSVO(system_properties_obj_.SSVO);
						this.UpdateHistroy();
						break;

					case SystemProperties.SP_SystemDisplayHDR:
						core_.DisplayHDR(system_properties_obj_.HDR);
						this.UpdateHistroy();
						break;

					case SystemProperties.SP_SystemDisplayAA:
						core_.DisplayAA(system_properties_obj_.AA);
						this.UpdateHistroy();
						break;

					case SystemProperties.SP_SystemDisplayGamma:
						core_.DisplayGamma(system_properties_obj_.Gamma);
						this.UpdateHistroy();
						break;

					case SystemProperties.SP_SystemDisplayColorGrading:
						core_.DisplayColorGrading(system_properties_obj_.ColorGrading);
						this.UpdateHistroy();
						break;

					default:
						break;
				}
			}
			else
			{
				var entity_common_props = properties.SelectedObject as EntityPropertyType;

				switch ((EntityProperties)item.PropertyOrder)
				{
					case EntityProperties.EP_EntityName:
						if (core_.EntityName(selected_entity_id_) != entity_common_props.Name)
						{
							this.ExecuteCommand(new KGEditorCommandSetEntityName(core_, selected_entity_id_, entity_common_props.Name));
						}
						break;

					case EntityProperties.EP_HideEntity:
						if (core_.HideEntity(selected_entity_id_) != entity_common_props.Hide)
						{
							this.ExecuteCommand(new KGEditorCommandHideEntity(core_, selected_entity_id_, entity_common_props.Hide));
						}
						break;

					case EntityProperties.EP_EntityTranslationX:
					case EntityProperties.EP_EntityTranslationY:
					case EntityProperties.EP_EntityTranslationZ:
						{
							float[] trans = new float[]
							{
								entity_common_props.TranslationX,
								entity_common_props.TranslationY,
								entity_common_props.TranslationZ
							};
							float[] old_trans = core_.EntityTranslation(selected_entity_id_);
							if (!this.FloatEqual(old_trans[0], trans[0]) || !this.FloatEqual(old_trans[1], trans[1])
								|| !this.FloatEqual(old_trans[2], trans[2]))
							{
								this.ExecuteCommand(new KGEditorCommandSetEntityTranslation(core_, selected_entity_id_, trans));
							}
						}
						break;

					case EntityProperties.EP_EntityRotationQuatX:
					case EntityProperties.EP_EntityRotationQuatY:
					case EntityProperties.EP_EntityRotationQuatZ:
					case EntityProperties.EP_EntityRotationQuatW:
						{
							float[] quat = new float[]
							{
								entity_common_props.RotationQuatX,
								entity_common_props.RotationQuatY,
								entity_common_props.RotationQuatZ,
								entity_common_props.RotationQuatW
							};
							float[] old_quat = core_.EntityRotation(selected_entity_id_);
							if (!this.FloatEqual(old_quat[0], quat[0]) || !this.FloatEqual(old_quat[1], quat[1])
								|| !this.FloatEqual(old_quat[2], quat[2]) || !this.FloatEqual(old_quat[3], quat[3]))
							{
								this.ExecuteCommand(new KGEditorCommandSetEntityRotation(core_, selected_entity_id_, quat));
							}

							float[] yaw_pitch_roll = this.QuatToYawPitchRoll(quat);
							entity_common_props.RotationYaw = yaw_pitch_roll[0];
							entity_common_props.RotationPitch = yaw_pitch_roll[1];
							entity_common_props.RotationRoll = yaw_pitch_roll[2];
						}
						break;

					case EntityProperties.EP_EntityRotationAngleYaw:
					case EntityProperties.EP_EntityRotationAnglePitch:
					case EntityProperties.EP_EntityRotationAngleRoll:
						{
							float[] quat = this.RotationQuatYawPitchRoll(entity_common_props.RotationYaw, entity_common_props.RotationPitch,
								entity_common_props.RotationRoll);
							float[] old_quat = core_.EntityRotation(selected_entity_id_);
							if (!this.FloatEqual(old_quat[0], quat[0]) || !this.FloatEqual(old_quat[1], quat[1])
								|| !this.FloatEqual(old_quat[2], quat[2]) || !this.FloatEqual(old_quat[3], quat[3]))
							{
								this.ExecuteCommand(new KGEditorCommandSetEntityRotation(core_, selected_entity_id_, quat));
							}
						}
						break;

					case EntityProperties.EP_EntityScalingX:
					case EntityProperties.EP_EntityScalingY:
					case EntityProperties.EP_EntityScalingZ:
						{
							float[] scaling = new float[]
							{
								entity_common_props.ScalingX,
								entity_common_props.ScalingY,
								entity_common_props.ScalingZ
							};
							float[] old_scale = core_.EntityScaling(selected_entity_id_);
							if (!this.FloatEqual(old_scale[0], scaling[0]) || !this.FloatEqual(old_scale[1], scaling[1])
								|| !this.FloatEqual(old_scale[2], scaling[2]))
							{
								this.ExecuteCommand(new KGEditorCommandSetEntityScaling(core_, selected_entity_id_, scaling));
							}
						}
						break;

					default:
						break;
				}

				if (properties.SelectedObject == light_properties_obj_)
				{
					switch ((LightProperties)item.PropertyOrder)
					{
						case LightProperties.LP_Type:
							break;

						case LightProperties.LP_Enabled:
							if (core_.LightEnabled(selected_entity_id_) != light_properties_obj_.Enabled)
							{
								this.ExecuteCommand(new KGEditorCommandSetLightEnabled(core_, selected_entity_id_,
									light_properties_obj_.Enabled));
							}
							break;

						case LightProperties.LP_AttributeNoShadow:
						case LightProperties.LP_AttributeNoDiffuse:
						case LightProperties.LP_AttributeNoSpecular:
						case LightProperties.LP_AttributeIndirect:
							{
								int attrib = 0;
								if (light_properties_obj_.NoShadow)
								{
									attrib |= (int)KGEditorCoreWrapper.LightSrcAttrib.LSA_NoShadow;
								}
								if (light_properties_obj_.NoDiffuse)
								{
									attrib |= (int)KGEditorCoreWrapper.LightSrcAttrib.LSA_NoDiffuse;
								}
								if (light_properties_obj_.NoSpecular)
								{
									attrib |= (int)KGEditorCoreWrapper.LightSrcAttrib.LSA_NoSpecular;
								}
								if (light_properties_obj_.Indirect)
								{
									attrib |= (int)KGEditorCoreWrapper.LightSrcAttrib.LSA_IndirectLighting;
								}

								if (core_.LightAttrib(selected_entity_id_) != attrib)
								{
									this.ExecuteCommand(new KGEditorCommandSetLightAttrib(core_, selected_entity_id_, attrib));
								}
							}
							break;

						case LightProperties.LP_Color:
						case LightProperties.LP_Multiplier:
							{
								float[] color = this.ColorToFloatPtr(light_properties_obj_.Color, light_properties_obj_.Multiplier);
								float[] old_color = core_.LightColor(selected_entity_id_);
								if (!this.FloatEqual(old_color[0], color[0]) || !this.FloatEqual(old_color[1], color[1])
									|| !this.FloatEqual(old_color[2], color[2]))
								{
									this.ExecuteCommand(new KGEditorCommandSetLightColor(core_, selected_entity_id_, color));
								}
							}
							break;

						case LightProperties.LP_FalloffConstant:
						case LightProperties.LP_FalloffLinear:
						case LightProperties.LP_FalloffQuadratic:
							{
								float[] falloff = new float[]
								{
									light_properties_obj_.FalloffConstant,
									light_properties_obj_.FalloffLinear,
									light_properties_obj_.FalloffQuadratic
								};
								float[] old_falloff = core_.LightFalloff(selected_entity_id_);
								if (!this.FloatEqual(old_falloff[0], falloff[0]) || !this.FloatEqual(old_falloff[1], falloff[1])
									|| !this.FloatEqual(old_falloff[2], falloff[2]))
								{
									this.ExecuteCommand(new KGEditorCommandSetLightFalloff(core_, selected_entity_id_, falloff));
								}
							}
							break;

						case LightProperties.LP_InnerAngle:
							{
								float inner_angle = (float)(core_.LightInnerAngle(selected_entity_id_) * (180 / Math.PI));
								if (!this.FloatEqual(inner_angle, light_properties_obj_.InnerAngle))
								{
									this.ExecuteCommand(new KGEditorCommandSetLightInnerAngle(core_, selected_entity_id_, inner_angle));
								}
							}
							break;

						case LightProperties.LP_OuterAngle:
							{
								float outer_angle = (float)(core_.LightOuterAngle(selected_entity_id_) * (180 / Math.PI));
								if (!this.FloatEqual(outer_angle, light_properties_obj_.OuterAngle))
								{
									this.ExecuteCommand(new KGEditorCommandSetLightOuterAngle(core_, selected_entity_id_, outer_angle));
								}
							}
							break;

						case LightProperties.LP_ProjectiveTex:
							if (core_.LightProjectiveTex(selected_entity_id_) != light_properties_obj_.ProjectiveTex)
							{
								this.ExecuteCommand(new KGEditorCommandSetProjectiveTex(core_, selected_entity_id_,
									light_properties_obj_.ProjectiveTex));
							}
							break;
					}
				}
				else if (properties.SelectedObject == camera_properties_obj_)
				{
					switch ((CameraProperties)item.PropertyOrder)
					{
						case CameraProperties.CP_LookAtX:
						case CameraProperties.CP_LookAtY:
						case CameraProperties.CP_LookAtZ:
							{
								float[] look_at = new float[3]
								{
									camera_properties_obj_.LookAtX,
									camera_properties_obj_.LookAtY,
									camera_properties_obj_.LookAtZ
								};
								float[] old_look_at = core_.CameraLookAt(selected_entity_id_);
								if (!this.FloatEqual(old_look_at[0], look_at[0]) || !this.FloatEqual(old_look_at[1], look_at[1])
									|| !this.FloatEqual(old_look_at[2], look_at[2]))
								{
									this.ExecuteCommand(new KGEditorCommandSetCameraLookAt(core_, selected_entity_id_, look_at));
								}
							}
							break;

						case CameraProperties.CP_UpVecX:
						case CameraProperties.CP_UpVecY:
						case CameraProperties.CP_UpVecZ:
							{
								float[] up_vec = new float[3]
								{
									camera_properties_obj_.UpVecX,
									camera_properties_obj_.UpVecY,
									camera_properties_obj_.UpVecZ
								};
								float[] old_up_vec = core_.CameraUpVec(selected_entity_id_);
								if (!this.FloatEqual(old_up_vec[0], up_vec[0]) || !this.FloatEqual(old_up_vec[1], up_vec[1])
									|| !this.FloatEqual(old_up_vec[2], up_vec[2]))
								{
									this.ExecuteCommand(new KGEditorCommandSetCameraUpVec(core_, selected_entity_id_, up_vec));
								}
							}
							break;

						case CameraProperties.CP_FoV:
							if (!this.FloatEqual(core_.CameraFoV(selected_entity_id_), camera_properties_obj_.FoV))
							{
								this.ExecuteCommand(new KGEditorCommandSetCameraFoV(core_, selected_entity_id_,
									camera_properties_obj_.FoV));
							}
							break;

						case CameraProperties.CP_Aspect:
							if (!this.FloatEqual(core_.CameraAspect(selected_entity_id_), camera_properties_obj_.Aspect))
							{
								this.ExecuteCommand(new KGEditorCommandSetCameraAspect(core_, selected_entity_id_,
									camera_properties_obj_.Aspect));
							}
							break;

						case CameraProperties.CP_NearPlane:
							if (!this.FloatEqual(core_.CameraNearPlane(selected_entity_id_), camera_properties_obj_.NearPlane))
							{
								this.ExecuteCommand(new KGEditorCommandSetCameraNearPlane(core_, selected_entity_id_,
									camera_properties_obj_.NearPlane));
							}
							break;

						case CameraProperties.CP_FarPlane:
							if (!this.FloatEqual(core_.CameraFarPlane(selected_entity_id_), camera_properties_obj_.FarPlane))
							{
								this.ExecuteCommand(new KGEditorCommandSetCameraFarPlane(core_, selected_entity_id_,
									camera_properties_obj_.FarPlane));
							}
							break;
					}
				}
			}
		}

		private void FileNameChanged(string name)
		{
			opened_file_ = name;
			int dot_offset = opened_file_.LastIndexOf('.');
			if (".kges" == opened_file_.Substring(dot_offset))
			{
				opened_file_ = opened_file_.Substring(0, dot_offset);
			}

			doc1.Title = opened_file_;
		}

		private string RelativePath(string name)
		{
			if ("" == name)
			{
				return "";
			}
			else if ("" == opened_file_)
			{
				return name;
			}
			else
			{
				Uri uri_kges = new Uri(opened_file_);
				Uri uri_tex = new Uri(name);
				Uri relative_uri = uri_kges.MakeRelativeUri(uri_tex);
				return relative_uri.ToString();
			}
		}

		private void UpdateHistroy()
		{
			undo.IsEnabled = (end_command_index_ > 0);
			redo.IsEnabled = (end_command_index_ < command_history_.Count);
		}

		private void EditorBGDragEnter(object sender, System.Windows.DragEventArgs e)
		{
			if (!e.Data.GetDataPresent(System.Windows.DataFormats.FileDrop) || (sender == e.Source))
			{
				e.Effects = System.Windows.DragDropEffects.None;
			}
		}

		private void EditorBGDrop(object sender, System.Windows.DragEventArgs e)
		{
			if (e.Data.GetDataPresent(System.Windows.DataFormats.FileDrop))
			{
				string[] files = (string[])e.Data.GetData(System.Windows.DataFormats.FileDrop);
				this.LoadScene(files[0]);
			}
		}

		public ReadOnlyCollection<SceneEntityViewModel> SceneRoot
		{
			get
			{
				return scene_entity_category_;
			}
		}

		string HistroyCmdName(int index)
		{
			return command_history_[index].Name();
		}

		private object ExecuteCommand(KGEditorCommand cmd)
		{
			object ret = cmd.Execute();
			int count = command_history_.Count - end_command_index_;
			if (count > 0)
			{
				command_history_.RemoveRange(end_command_index_, count);
			}
			++ end_command_index_;
			command_history_.Add(cmd);

			this.UpdateHistroy();

			return ret;
		}

		private void Undo()
		{
			//Assert(end_command_index_ != 0);

			-- end_command_index_;
			command_history_[end_command_index_].Revoke();
		}

		private void Redo()
		{
			//Assert(end_command_index_ != command_history_.Count);

			command_history_[end_command_index_].Execute();
			++ end_command_index_;
		}

		private void ClearHistroy()
		{
			command_history_.Clear();
			end_command_index_ = 0;
		}

		float LinearToSRGB(float linear)
		{
			if (linear < 0.0031308f)
			{
				return 12.92f * linear;
			}
			else
			{
				const float ALPHA = 0.055f;
				return (1 + ALPHA) * (float)Math.Pow(linear, 1 / 2.4f) - ALPHA;
			}
		}

		float SRGBToLinear(float srgb)
		{
			if (srgb < 0.04045f)
			{
				return srgb / 12.92f;
			}
			else
			{
				const float ALPHA = 0.055f;
				return (float)Math.Pow((srgb + ALPHA) / (1 + ALPHA), 2.4f);
			}
		}

		private float FloatPtrToMultipiler(float[] clr)
		{
			return Math.Max(Math.Max(Math.Max(clr[0], clr[1]), clr[2]), 1.0f);
		}

		private Color FloatPtrToLDRColor(float[] clr, float multiplier)
		{
			float[] temp = new float[3];
			for (int i = 0; i < 3; ++ i)
			{
				temp[i] = LinearToSRGB(clr[i] / multiplier);
			}
			return Color.FromArgb(255,
				(byte)(Math.Max(Math.Min((int)(temp[0] * 255 + 0.5f), 255), 0)),
				(byte)(Math.Max(Math.Min((int)(temp[1] * 255 + 0.5f), 255), 0)),
				(byte)(Math.Max(Math.Min((int)(temp[2] * 255 + 0.5f), 255), 0)));
		}

		private float[] ColorToFloatPtr(Color clr, float multiplier)
		{
			float[] ret = new float[3];
			ret[0] = clr.R / 255.0f;
			ret[1] = clr.G / 255.0f;
			ret[2] = clr.B / 255.0f;
			for (int i = 0; i < 3; ++ i)
			{
				ret[i] = SRGBToLinear(ret[i]) * multiplier;
			}
			return ret;
		}

		// From http://www.euclideanspace.com/maths/geometry/rotations/conversions/quaternionToEuler/index.htm
		float[] QuatToYawPitchRoll(float[] quat)
		{
			float[] yaw_pitch_roll = new float[3];

			float sqx = quat[0] * quat[0];
			float sqy = quat[1] * quat[1];
			float sqz = quat[2] * quat[2];
			float sqw = quat[3] * quat[3];
			float unit = sqx + sqy + sqz + sqw;
			float test = quat[3] * quat[0] + quat[1] * quat[2];
			if (test > 0.499f * unit)
			{
				// singularity at north pole
				yaw_pitch_roll[0] = 2 * (float)Math.Atan2(quat[2], quat[3]);
				yaw_pitch_roll[1] = (float)Math.PI / 2;
				yaw_pitch_roll[2] = 0;
			}
			else if (test< -0.499f * unit)
			{
				// singularity at south pole
				yaw_pitch_roll[0] = -2 * (float)Math.Atan2(quat[2], quat[3]);
				yaw_pitch_roll[1] = -(float)Math.PI / 2;
				yaw_pitch_roll[2] = 0;
			}
			else
			{
				yaw_pitch_roll[0] = (float)Math.Atan2(2 * (quat[1] * quat[3] - quat[0] * quat[2]), -sqx - sqy + sqz + sqw);
				yaw_pitch_roll[1] = (float)Math.Asin(2 * test / unit);
				yaw_pitch_roll[2] = (float)Math.Atan2(2 * (quat[2] * quat[3] - quat[0] * quat[1]), -sqx + sqy - sqz + sqw);
			}

			return yaw_pitch_roll;
		}

		float[] RotationQuatYawPitchRoll(float yaw, float pitch, float roll)
		{
			float ang_x = pitch / 2;
			float ang_y = yaw / 2;
			float ang_z = roll / 2;
			float sx = (float)Math.Sin(ang_x);
			float cx = (float)Math.Cos(ang_x);
			float sy = (float)Math.Sin(ang_y);
			float cy = (float)Math.Cos(ang_y);
			float sz = (float)Math.Sin(ang_z);
			float cz = (float)Math.Cos(ang_z);

			return new float[]
			{
				sx * cy * cz + cx * sy * sz,
				cx * sy * cz - sx * cy * sz,
				cx * cy * sz - sx * sy * cz,
				sx * sy * sz + cx * cy * cz
			};
		}

		private bool FloatEqual(float a, float b)
		{
			return Math.Abs(a - b) < 1e-6f;
		}

		private KGEditorCoreWrapper core_;
		private SystemPropertyType system_properties_obj_ = new SystemPropertyType();
		private EntityPropertyType entity_properties_obj_ = new EntityPropertyType();
		private LightPropertyType light_properties_obj_ = new LightPropertyType();
		private CameraPropertyType camera_properties_obj_ = new CameraPropertyType();
		private KGEditorCoreWrapper.ControlMode control_mode_ = KGEditorCoreWrapper.ControlMode.CM_EntitySelection;
		private uint selected_entity_id_ = 0;
		private string opened_file_ = "";

		private readonly ReadOnlyCollection<SceneEntityViewModel> scene_entity_category_;

		private List<KGEditorCommand> command_history_ = new List<KGEditorCommand>();
		private int end_command_index_ = 0;
	}

	public class CameraItemsSource : IItemsSource
	{
		static public Xceed.Wpf.Toolkit.PropertyGrid.Attributes.ItemCollection items
			= new Xceed.Wpf.Toolkit.PropertyGrid.Attributes.ItemCollection();

		public Xceed.Wpf.Toolkit.PropertyGrid.Attributes.ItemCollection GetValues()
		{
			return items;
		}
	}

	public class LightTypeItemsSource : IItemsSource
	{
		static public Xceed.Wpf.Toolkit.PropertyGrid.Attributes.ItemCollection items
			= new Xceed.Wpf.Toolkit.PropertyGrid.Attributes.ItemCollection()
		{
			"Ambient",
			"Sun",
			"Directional",
			"Point",
			"Spot",
			"SphereArea",
			"TubeArea"
		};

		public Xceed.Wpf.Toolkit.PropertyGrid.Attributes.ItemCollection GetValues()
		{
			return items;
		}
	}

	public class RadioButtonCheckedConverter : IValueConverter
	{
		public object Convert(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
		{
			if (value == null)
			{
				return false;
			}
			return value.Equals(parameter);
		}

		public object ConvertBack(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
		{
			return value.Equals(true) ? parameter : System.Windows.Data.Binding.DoNothing;
		}
	}
}
