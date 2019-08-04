using KlayGE;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Diagnostics;
using System.Linq;
using System.Windows;
using System.Windows.Controls.Ribbon;
using System.Windows.Data;
using System.Windows.Forms;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using Xceed.Wpf.Toolkit.PropertyGrid.Attributes;

namespace KGEditor
{
	public enum ControlModeEnum
	{
		CM_EntitySelection = KGEditorCoreWrapper.ControlMode.CM_EntitySelection,
		CM_EntityPosition = KGEditorCoreWrapper.ControlMode.CM_EntityPosition,
		CM_EntityRotation = KGEditorCoreWrapper.ControlMode.CM_EntityRotation,
		CM_EntityScale = KGEditorCoreWrapper.ControlMode.CM_EntityScale,
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
			EP_EntityVisible,
			EP_EntityPositionX,
			EP_EntityPositionY,
			EP_EntityPositionZ,
			EP_EntityRotationQuatX,
			EP_EntityRotationQuatY,
			EP_EntityRotationQuatZ,
			EP_EntityRotationQuatW,
			EP_EntityRotationAngleYaw,
			EP_EntityRotationAnglePitch,
			EP_EntityRotationAngleRoll,
			EP_EntityScaleX,
			EP_EntityScaleY,
			EP_EntityScaleZ,

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
		[CategoryOrder("Position", 1)]
		[CategoryOrder("Rotation", 2)]
		[CategoryOrder("Scale", 3)]
		public class EntityPropertyType
		{
			[Category("Properties")]
			[DisplayName("Name")]
			[PropertyOrder((int)EntityProperties.EP_EntityName)]
			public string Name { get; set; }

			[Category("Properties")]
			[DisplayName("Visible")]
			[PropertyOrder((int)EntityProperties.EP_EntityVisible)]
			public bool Visible { get; set; }

			[Category("Position")]
			[DisplayName("X")]
			[PropertyOrder((int)EntityProperties.EP_EntityPositionX)]
			public float PositionX { get; set; }

			[Category("Position")]
			[DisplayName("Y")]
			[PropertyOrder((int)EntityProperties.EP_EntityPositionY)]
			public float PositionY { get; set; }

			[Category("Position")]
			[DisplayName("Z")]
			[PropertyOrder((int)EntityProperties.EP_EntityPositionZ)]
			public float PositionZ { get; set; }

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

			[Category("Scale")]
			[DisplayName("X")]
			[PropertyOrder((int)EntityProperties.EP_EntityScaleX)]
			public float ScaleX { get; set; }

			[Category("Scale")]
			[DisplayName("Y")]
			[PropertyOrder((int)EntityProperties.EP_EntityScaleY)]
			public float ScaleY { get; set; }

			[Category("Scale")]
			[DisplayName("Z")]
			[PropertyOrder((int)EntityProperties.EP_EntityScaleZ)]
			public float ScaleZ { get; set; }
		}

		[CategoryOrder("Properties", 0)]
		[CategoryOrder("Position", 1)]
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
		[CategoryOrder("Position", 1)]
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

			scene_ = new Scene(this);

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

			core_.UpdatePropertyCallback(this.UpdatePropListNativeCallback);
			core_.UpdateSelectEntityCallback(this.UpdateSelectEntityNativeCallback);
			core_.AddModelCallback(this.AddModelNativeCallback);
			core_.AddLightCallback(this.AddLightNativeCallback);
			core_.AddCameraCallback(this.AddCameraNativeCallback);

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

		public static KGEditorCoreWrapper KGEditNativeCore
		{
			get
			{
				return core_;
			}
		}

		private void LoadScene(string file_name)
		{
			string ext_name = System.IO.Path.GetExtension(file_name).ToLower();
			if (ext_name != ".kges")
			{
				return;
			}

			core_.LoadScene(file_name);
			scene_.RetrieveProperties();

			this.FileNameChanged(file_name);

			this.ClearHistroy();
			this.UpdateHistroy();

			save.IsEnabled = true;
			save_as.IsEnabled = true;

			this.SelectedEntityId = 0;
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
			dlg.Filter = "All Model Files|*.meshml;*.model_bin;*.3ds;*.ac;*.ase;*.assbin;*.assxml;*.b3d;*.bvh;*.dae;*.dxf;*.csm;"
				+ "*.hmp;*.irr;*.lwo;*.lws;*.md2;*.md3;*.md5mesh;*.mdc;*.mdl;*.nff;*.ndo;*.off;*.obj;*.ogre;*.opengex;*.ply;*.ms3d;*.cob;"
				+ "*.blend;*.ifc;*.xgl;*.fbx;*.q3d;*.q3bsp;*.raw;*.smd;*.stl;*.terragen;*.3d;*.x;*.gltf;*.glb|All Files|*.*";
			dlg.CheckPathExists = true;
			dlg.CheckFileExists = true;
			if (true == dlg.ShowDialog())
			{
				this.AddModelNativeCallback(dlg.FileName);

				save.IsEnabled = true;
				save_as.IsEnabled = true;
			}
		}

		private void AddAmbientLightClick(object sender, RoutedEventArgs e)
		{
			this.AddLightNativeCallback(KGEditorCoreWrapper.LightType.LT_Ambient);

			save.IsEnabled = true;
			save_as.IsEnabled = true;
		}

		private void AddDirectionalLightClick(object sender, RoutedEventArgs e)
		{
			this.AddLightNativeCallback(KGEditorCoreWrapper.LightType.LT_Directional);

			save.IsEnabled = true;
			save_as.IsEnabled = true;
		}

		private void AddPointLightClick(object sender, RoutedEventArgs e)
		{
			this.AddLightNativeCallback(KGEditorCoreWrapper.LightType.LT_Point);

			save.IsEnabled = true;
			save_as.IsEnabled = true;
		}

		private void AddSpotLightClick(object sender, RoutedEventArgs e)
		{
			this.AddLightNativeCallback(KGEditorCoreWrapper.LightType.LT_Spot);

			save.IsEnabled = true;
			save_as.IsEnabled = true;
		}

		private void AddCameraClick(object sender, RoutedEventArgs e)
		{
			this.AddCameraNativeCallback();

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
				var mode = (KGEditorCoreWrapper.ControlMode)value;
				if (mode != control_mode_)
				{
					this.ExecuteCommand(new KGEditorCommandSetControlMode(this, mode));
				}
			}
		}

		private void AddModelNativeCallback(string file_name)
		{
			this.ExecuteCommand(new KGEditorCommandAddModel(core_, this, scene_, file_name));
		}

		private void AddLightNativeCallback(KGEditorCoreWrapper.LightType type)
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
				foreach (var light in scene_.SceneRoot[1].Children)
				{
					if (light.Entity.Name == light_name)
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

			this.ExecuteCommand(new KGEditorCommandAddLight(core_, this, scene_, type, light_name));
		}

		private void AddCameraNativeCallback()
		{
			string camera_name;
			int index = 0;
			for (;;)
			{
				camera_name = "Camera" + index.ToString();
				bool found = false;
				foreach (var camera in scene_.SceneRoot[2].Children)
				{
					if (camera.Entity.Name == camera_name)
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

			this.ExecuteCommand(new KGEditorCommandAddCamera(core_, this, scene_, camera_name));
		}

		private void UndoClick(object sender, RoutedEventArgs e)
		{
			this.Undo();

			this.UpdateEntityProperties();
			this.UpdateHistroy();
		}

		private void RedoClick(object sender, RoutedEventArgs e)
		{
			this.Redo();

			this.UpdateEntityProperties();
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
			if (this.SelectedEntityId != entity_id)
			{
				this.ExecuteCommand(new KGEditorCommandSelectEntity(this, entity_id));
			}
		}

		public void RemoveEntity(uint entity_id)
		{
			if (entity_id > 0)
			{
				foreach (var category in scene_.SceneRoot)
				{
					foreach (var entity in category.Children)
					{
						if (entity.Entity.Id == entity_id)
						{
							this.ExecuteCommand(new KGEditorCommandRemoveEntity(core_, this, entity.Entity));
							category.Children.Remove(entity);
							break;
						}
					}
				}
			}
		}

		public void RestoreEntityInfo(SceneEntity entity)
		{
			int type = (int)entity.Type;
			scene_.SceneRoot[type].Children.Add(new SceneEntityViewModel(this, entity));
			scene_.SceneRoot[type].Children.Last().SelectedInternal(true);
		}

		public void DoSetControlMode(KGEditorCoreWrapper.ControlMode mode)
		{
			control_mode_ = mode;
			core_.SetControlMode(control_mode_);
		}

		public uint SelectedEntityId
		{
			get
			{
				return selected_entity_id_;
			}
			set
			{
				if (selected_entity_id_ != value)
				{
					selected_entity_id_ = value;
					core_.SelectEntity(selected_entity_id_);
					this.UpdateEntityProperties();
				}
			}
		}

		private void UpdateEntityProperties()
		{
			properties.SelectedObject = null;

			if (0 == selected_entity_id_)
			{
				properties.SelectedObject = system_properties_obj_;
			}
			else
			{
				var entity = scene_.FindSceneEntity(selected_entity_id_);
				switch (entity.Entity.Type)
				{
					case SceneEntityType.ET_Model:
						properties.SelectedObject = entity_properties_obj_;
						break;

					case SceneEntityType.ET_Light:
						properties.SelectedObject = light_properties_obj_;
						break;

					case SceneEntityType.ET_Camera:
					default:
						properties.SelectedObject = camera_properties_obj_;
						break;
				}
			}
		}

		private void UpdatePropListNativeCallback()
		{
			int active_prop_list = -1;

			if (core_ != null)
			{
				if (selected_entity_id_ > 0)
				{
					var entity = scene_.FindSceneEntity(selected_entity_id_).Entity;

					entity.RetrieveProperties();
					float[] yaw_pitch_roll = MathHelper.QuatToYawPitchRoll(entity.TransformRotation);

					active_prop_list = (int)entity.Type;

					EntityPropertyType entity_common_props;
					switch (entity.Type)
					{
						case SceneEntityType.ET_Model:
							entity_common_props = entity_properties_obj_;
							break;

						case SceneEntityType.ET_Light:
							entity_common_props = light_properties_obj_;
							break;

						case SceneEntityType.ET_Camera:
						default:
							entity_common_props = camera_properties_obj_;
							break;
					}

					entity_common_props.Name = entity.Name;
					entity_common_props.Visible = entity.Visible;
					entity_common_props.PositionX = entity.TransformPosition[0];
					entity_common_props.PositionY = entity.TransformPosition[1];
					entity_common_props.PositionZ = entity.TransformPosition[2];
					entity_common_props.RotationQuatX = entity.TransformRotation[0];
					entity_common_props.RotationQuatY = entity.TransformRotation[1];
					entity_common_props.RotationQuatZ = entity.TransformRotation[2];
					entity_common_props.RotationQuatW = entity.TransformRotation[3];
					entity_common_props.RotationYaw = yaw_pitch_roll[0];
					entity_common_props.RotationPitch = yaw_pitch_roll[1];
					entity_common_props.RotationRoll = yaw_pitch_roll[2];
					entity_common_props.ScaleX = entity.TransformScale[0];
					entity_common_props.ScaleY = entity.TransformScale[1];
					entity_common_props.ScaleZ = entity.TransformScale[2];

					switch (entity.Type)
					{
						case SceneEntityType.ET_Model:
							break;

						case SceneEntityType.ET_Light:
							{
								var light = entity as SceneEntityLight;
								Debug.Assert(light != null);

								light_properties_obj_.Type
									= LightTypeItemsSource.items[(int)light.LightType].Value as string;
								light_properties_obj_.Enabled = light.LightEnabled;
								light_properties_obj_.NoShadow
									= (light.LightAttrib & (int)KGEditorCoreWrapper.LightSrcAttrib.LSA_NoShadow) != 0;
								light_properties_obj_.NoDiffuse
									= (light.LightAttrib & (int)KGEditorCoreWrapper.LightSrcAttrib.LSA_NoDiffuse) != 0;
								light_properties_obj_.NoSpecular
									= (light.LightAttrib & (int)KGEditorCoreWrapper.LightSrcAttrib.LSA_NoSpecular) != 0;
								light_properties_obj_.Indirect
									= (light.LightAttrib & (int)KGEditorCoreWrapper.LightSrcAttrib.LSA_IndirectLighting) != 0;
								light_properties_obj_.Multiplier = MathHelper.FloatPtrToMultipiler(light.LightColor);
								light_properties_obj_.Color = MathHelper.FloatPtrToLDRColor(light.LightColor,
									light_properties_obj_.Multiplier);
								light_properties_obj_.FalloffConstant = light.LightFalloff[0];
								light_properties_obj_.FalloffLinear = light.LightFalloff[1];
								light_properties_obj_.FalloffQuadratic = light.LightFalloff[2];
								light_properties_obj_.InnerAngle = MathHelper.Rad2Deg(light.LightInnerAngle);
								light_properties_obj_.OuterAngle = MathHelper.Rad2Deg(light.LightOuterAngle);
								light_properties_obj_.ProjectiveTex = light.LightProjectiveTex;
							}
							break;

						case SceneEntityType.ET_Camera:
						default:
							{
								var camera = entity as SceneEntityCamera;
								Debug.Assert(camera != null);

								camera_properties_obj_.LookAtX = camera.CameraLookAt[0];
								camera_properties_obj_.LookAtY = camera.CameraLookAt[1];
								camera_properties_obj_.LookAtZ = camera.CameraLookAt[2];
								camera_properties_obj_.UpVecX = camera.CameraUpVec[0];
								camera_properties_obj_.UpVecY = camera.CameraUpVec[1];
								camera_properties_obj_.UpVecZ = camera.CameraUpVec[2];

								camera_properties_obj_.FoV = camera.CameraFoV;
								camera_properties_obj_.Aspect = camera.CameraAspect;
								camera_properties_obj_.NearPlane = camera.CameraNearPlane;
								camera_properties_obj_.FarPlane = camera.CameraFarPlane;
							}
							break;
					}
				}
				else
				{
					system_properties_obj_.SceneName = scene_.Name;
					system_properties_obj_.SkyBox = scene_.SkyboxName;
				}
			}

			properties.SelectedObject = null;

			switch ((SceneEntityType)active_prop_list)
			{
				case SceneEntityType.ET_Model:
					properties.SelectedObject = entity_properties_obj_;
					break;

				case SceneEntityType.ET_Light:
					properties.SelectedObject = light_properties_obj_;
					break;

				case SceneEntityType.ET_Camera:
					properties.SelectedObject = camera_properties_obj_;
					break;

				default:
					properties.SelectedObject = system_properties_obj_;

					CameraItemsSource.items.Clear();
					CameraItemsSource.items.Add("System");

					foreach (var camera in scene_.SceneRoot[2].Children)
					{
						CameraItemsSource.items.Add(camera.Entity.Name);
					}
					break;
			}
		}

		private void UpdateSelectEntityNativeCallback(uint entity_id)
		{
			if (selected_entity_id_ != entity_id)
			{
				if (selected_entity_id_ > 0)
				{
					scene_.FindSceneEntity(selected_entity_id_).IsSelected = false;
				}

				if (entity_id > 0)
				{
					scene_.FindSceneEntity(entity_id).IsSelected = true;
				}
				else
				{
					this.SelectEntity(0);
				}
			}
		}

		private void PropertyGridValueChanged(object sender, Xceed.Wpf.Toolkit.PropertyGrid.PropertyValueChangedEventArgs e)
		{
			var item = e.OriginalSource as Xceed.Wpf.Toolkit.PropertyGrid.PropertyItem;
			if (properties.SelectedObject == system_properties_obj_)
			{
				switch ((SystemProperties)item.PropertyOrder)
				{
					case SystemProperties.SP_SystemSceneName:
						if (scene_.Name != system_properties_obj_.SceneName)
						{
							this.ExecuteCommand(new KGEditorCommandSetSceneName(scene_, system_properties_obj_.SceneName));
						}
						break;

					case SystemProperties.SP_SystemActiveCamera:
						{
							if (system_properties_obj_.ActiveCamera == "System")
							{
								this.ExecuteCommand(new KGEditorCommandSetActiveCamera(scene_, 0));
							}
							else
							{
								foreach (var camera in scene_.SceneRoot[2].Children)
								{
									if (system_properties_obj_.ActiveCamera == camera.Entity.Name)
									{
										if (scene_.ActiveCameraId != camera.Entity.Id)
										{
											this.ExecuteCommand(new KGEditorCommandSetActiveCamera(scene_, camera.Entity.Id));
										}
										break;
									}
								}
							}
						}
						break;

					case SystemProperties.SP_SystemSkyBox:
						{
							string skybox_name = this.RelativePath(system_properties_obj_.SkyBox);
							if (scene_.SkyboxName != skybox_name)
							{
								this.ExecuteCommand(new KGEditorCommandSetSkyboxName(scene_, skybox_name));
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
				var entity = scene_.FindSceneEntity(selected_entity_id_).Entity;

				switch ((EntityProperties)item.PropertyOrder)
				{
					case EntityProperties.EP_EntityName:
						if (entity.Name != entity_common_props.Name)
						{
							this.ExecuteCommand(new KGEditorCommandSetEntityName(entity, entity_common_props.Name));
						}
						break;

					case EntityProperties.EP_EntityVisible:
						if (entity.Visible != entity_common_props.Visible)
						{
							this.ExecuteCommand(new KGEditorCommandEntityVisible(entity, entity_common_props.Visible));
						}
						break;

					case EntityProperties.EP_EntityPositionX:
					case EntityProperties.EP_EntityPositionY:
					case EntityProperties.EP_EntityPositionZ:
						{
							float[] trans = new float[]
							{
								entity_common_props.PositionX,
								entity_common_props.PositionY,
								entity_common_props.PositionZ
							};
							float[] old_trans = entity.TransformPosition;
							if (!MathHelper.FloatArrayEqual(old_trans, trans))
							{
								this.ExecuteCommand(new KGEditorCommandSetEntityPosition(entity, trans));
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
							float[] old_quat = entity.TransformRotation;
							if (!MathHelper.FloatArrayEqual(old_quat, quat))
							{
								this.ExecuteCommand(new KGEditorCommandSetEntityRotation(entity, quat));
							}

							float[] yaw_pitch_roll = MathHelper.QuatToYawPitchRoll(quat);
							entity_common_props.RotationYaw = yaw_pitch_roll[0];
							entity_common_props.RotationPitch = yaw_pitch_roll[1];
							entity_common_props.RotationRoll = yaw_pitch_roll[2];
						}
						break;

					case EntityProperties.EP_EntityRotationAngleYaw:
					case EntityProperties.EP_EntityRotationAnglePitch:
					case EntityProperties.EP_EntityRotationAngleRoll:
						{
							float[] quat = MathHelper.RotationQuatYawPitchRoll(entity_common_props.RotationYaw,
								entity_common_props.RotationPitch, entity_common_props.RotationRoll);
							float[] old_quat = entity.TransformRotation;
							if (!MathHelper.FloatArrayEqual(old_quat, quat))
							{
								this.ExecuteCommand(new KGEditorCommandSetEntityRotation(entity, quat));
							}
						}
						break;

					case EntityProperties.EP_EntityScaleX:
					case EntityProperties.EP_EntityScaleY:
					case EntityProperties.EP_EntityScaleZ:
						{
							float[] scale = new float[]
							{
								entity_common_props.ScaleX,
								entity_common_props.ScaleY,
								entity_common_props.ScaleZ
							};
							float[] old_scale = entity.TransformScale;
							if (!MathHelper.FloatArrayEqual(old_scale, scale))
							{
								this.ExecuteCommand(new KGEditorCommandSetEntityScale(entity, scale));
							}
						}
						break;

					default:
						break;
				}

				if (properties.SelectedObject == light_properties_obj_)
				{
					var light = entity as SceneEntityLight;
					Debug.Assert(light != null);

					switch ((LightProperties)item.PropertyOrder)
					{
						case LightProperties.LP_Type:
							break;

						case LightProperties.LP_Enabled:
							if (light.LightEnabled != light_properties_obj_.Enabled)
							{
								this.ExecuteCommand(new KGEditorCommandSetLightEnabled(light, light_properties_obj_.Enabled));
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

								if (light.LightAttrib != attrib)
								{
									this.ExecuteCommand(new KGEditorCommandSetLightAttrib(light, attrib));
								}
							}
							break;

						case LightProperties.LP_Color:
						case LightProperties.LP_Multiplier:
							{
								float[] color = MathHelper.ColorToFloatPtr(light_properties_obj_.Color, light_properties_obj_.Multiplier);
								float[] old_color = light.LightColor;
								if (!MathHelper.FloatArrayEqual(old_color, color))
								{
									this.ExecuteCommand(new KGEditorCommandSetLightColor(light, color));
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
								float[] old_falloff = light.LightFalloff;
								if (!MathHelper.FloatArrayEqual(old_falloff, falloff))
								{
									this.ExecuteCommand(new KGEditorCommandSetLightFalloff(light, falloff));
								}
							}
							break;

						case LightProperties.LP_InnerAngle:
							{
								float inner_angle = MathHelper.Deg2Rad(light_properties_obj_.InnerAngle);
								if (!MathHelper.FloatEqual(light.LightInnerAngle, inner_angle))
								{
									this.ExecuteCommand(new KGEditorCommandSetLightInnerAngle(light, inner_angle));
								}
							}
							break;

						case LightProperties.LP_OuterAngle:
							{
								float outer_angle = MathHelper.Deg2Rad(light_properties_obj_.OuterAngle);
								if (!MathHelper.FloatEqual(light.LightOuterAngle, outer_angle))
								{
									this.ExecuteCommand(new KGEditorCommandSetLightOuterAngle(light, outer_angle));
								}
							}
							break;

						case LightProperties.LP_ProjectiveTex:
							if (light.LightProjectiveTex != light_properties_obj_.ProjectiveTex)
							{
								this.ExecuteCommand(new KGEditorCommandSetProjectiveTex(light, light_properties_obj_.ProjectiveTex));
							}
							break;
					}
				}
				else if (properties.SelectedObject == camera_properties_obj_)
				{
					var camera = entity as SceneEntityCamera;
					Debug.Assert(camera != null);

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
								float[] old_look_at = camera.CameraLookAt;
								if (!MathHelper.FloatArrayEqual(old_look_at, look_at))
								{
									this.ExecuteCommand(new KGEditorCommandSetCameraLookAt(camera, look_at));
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
								float[] old_up_vec = camera.CameraUpVec;
								if (!MathHelper.FloatArrayEqual(old_up_vec, up_vec))
								{
									this.ExecuteCommand(new KGEditorCommandSetCameraUpVec(camera, up_vec));
								}
							}
							break;

						case CameraProperties.CP_FoV:
							if (!MathHelper.FloatEqual(camera.CameraFoV, camera_properties_obj_.FoV))
							{
								this.ExecuteCommand(new KGEditorCommandSetCameraFoV(camera, camera_properties_obj_.FoV));
							}
							break;

						case CameraProperties.CP_Aspect:
							if (!MathHelper.FloatEqual(camera.CameraAspect, camera_properties_obj_.Aspect))
							{
								this.ExecuteCommand(new KGEditorCommandSetCameraAspect(camera, camera_properties_obj_.Aspect));
							}
							break;

						case CameraProperties.CP_NearPlane:
							if (!MathHelper.FloatEqual(camera.CameraNearPlane, camera_properties_obj_.NearPlane))
							{
								this.ExecuteCommand(new KGEditorCommandSetCameraNearPlane(camera, camera_properties_obj_.NearPlane));
							}
							break;

						case CameraProperties.CP_FarPlane:
							if (!MathHelper.FloatEqual(camera.CameraFarPlane, camera_properties_obj_.FarPlane))
							{
								this.ExecuteCommand(new KGEditorCommandSetCameraFarPlane(camera, camera_properties_obj_.FarPlane));
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
				return scene_.SceneRoot;
			}
		}

		private string HistroyCmdName(int index)
		{
			return command_history_[index].Name;
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
			Debug.Assert(end_command_index_ != 0);

			-- end_command_index_;
			command_history_[end_command_index_].Revoke();
		}

		private void Redo()
		{
			Debug.Assert(end_command_index_ != command_history_.Count);

			command_history_[end_command_index_].Execute();
			++ end_command_index_;
		}

		private void ClearHistroy()
		{
			command_history_.Clear();
			end_command_index_ = 0;
		}

		private static KGEditorCoreWrapper core_;
		private SystemPropertyType system_properties_obj_ = new SystemPropertyType();
		private EntityPropertyType entity_properties_obj_ = new EntityPropertyType();
		private LightPropertyType light_properties_obj_ = new LightPropertyType();
		private CameraPropertyType camera_properties_obj_ = new CameraPropertyType();
		private KGEditorCoreWrapper.ControlMode control_mode_ = KGEditorCoreWrapper.ControlMode.CM_EntitySelection;
		private uint selected_entity_id_ = 0;
		private string opened_file_ = "";

		private Scene scene_;

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
