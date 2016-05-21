using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using System.ComponentModel;
using System.Windows.Controls.Ribbon;
using Xceed.Wpf.Toolkit.PropertyGrid.Attributes;
using System.Windows.Forms;

namespace MtlEditor
{
	/// <summary>
	/// Interaction logic for MainWindow.xaml
	/// </summary>
	public partial class MainWindow : RibbonWindow
	{
		enum PropertyOrders
		{
			PO_Meshes = 0,
			PO_VertexStreams,
			PO_Ambient,
			PO_Diffuse,
			PO_Specular,
			PO_Shininess,
			PO_Emit,
			PO_Opacity,
			PO_DiffuseTex,
			PO_SpecularTex,
			PO_ShininessTex,
			PO_NormalTex,
			PO_HeightTex,
			PO_EmitTex,
			PO_OpacityTex,
			PO_DetailMode,
			PO_HeightOffset,
			PO_HeightScale,
			PO_EdgeTessHint,
			PO_InsideTessHint,
			PO_MinTess,
			PO_MaxTess
		};

		[CategoryOrder("Meshes", 0)]
		[CategoryOrder("Vertex Streams", 1)]
		[CategoryOrder("Material", 2)]
		[CategoryOrder("Textures", 3)]
		public class ModelPropertyTypes
		{
			[Category("Meshes")]
			[DisplayName("Meshes")]
			[ItemsSource(typeof(MeshItemsSource))]
			[PropertyOrder((int)PropertyOrders.PO_Meshes)]
			public string meshes { get; set; }

			[Category("Vertex Streams")]
			[DisplayName("Vertex Streams")]
			[PropertyOrder((int)PropertyOrders.PO_VertexStreams)]
			public List<string> vertex_streams { get; set; }

			[Category("Material")]
			[DisplayName("Ambient")]
			[PropertyOrder((int)PropertyOrders.PO_Ambient)]
			public Color ambient { get; set; }
			[Category("Material")]
			[DisplayName("Diffuse")]
			[PropertyOrder((int)PropertyOrders.PO_Diffuse)]
			public Color diffuse { get; set; }
			[Category("Material")]
			[DisplayName("Specular")]
			[PropertyOrder((int)PropertyOrders.PO_Specular)]
			public Color specular { get; set; }
			[Category("Material")]
			[DisplayName("Shininess")]
			[PropertyOrder((int)PropertyOrders.PO_Shininess)]
			public float shininess { get; set; }
			[Category("Material")]
			[DisplayName("Emit")]
			[PropertyOrder((int)PropertyOrders.PO_Emit)]
			public Color emit { get; set; }
			[Category("Material")]
			[DisplayName("Opacity")]
			[PropertyOrder((int)PropertyOrders.PO_Opacity)]
			[Editor(typeof(SliderUserControlEditor), typeof(SliderUserControlEditor))]
			public float opacity { get; set; }

			[Category("Textures")]
			[DisplayName("Diffuse")]
			[PropertyOrder((int)PropertyOrders.PO_DiffuseTex)]
			[Editor(typeof(OpenTexUserControlEditor), typeof(OpenTexUserControlEditor))]
			public string diffuse_tex { get; set; }
			[Category("Textures")]
			[DisplayName("Specular")]
			[PropertyOrder((int)PropertyOrders.PO_SpecularTex)]
			[Editor(typeof(OpenTexUserControlEditor), typeof(OpenTexUserControlEditor))]
			public string specular_tex { get; set; }
			[Category("Textures")]
			[DisplayName("Shininess")]
			[PropertyOrder((int)PropertyOrders.PO_ShininessTex)]
			[Editor(typeof(OpenTexUserControlEditor), typeof(OpenTexUserControlEditor))]
			public string shininess_tex { get; set; }
			[Category("Textures")]
			[DisplayName("Normal")]
			[PropertyOrder((int)PropertyOrders.PO_NormalTex)]
			[Editor(typeof(OpenTexUserControlEditor), typeof(OpenTexUserControlEditor))]
			public string normal_tex { get; set; }
			[Category("Textures")]
			[DisplayName("Height")]
			[PropertyOrder((int)PropertyOrders.PO_HeightTex)]
			[Editor(typeof(OpenTexUserControlEditor), typeof(OpenTexUserControlEditor))]
			public string height_tex { get; set; }
			[Category("Textures")]
			[DisplayName("Emit")]
			[PropertyOrder((int)PropertyOrders.PO_EmitTex)]
			[Editor(typeof(OpenTexUserControlEditor), typeof(OpenTexUserControlEditor))]
			public string emit_tex { get; set; }
			[Category("Textures")]
			[DisplayName("Opacity")]
			[PropertyOrder((int)PropertyOrders.PO_OpacityTex)]
			[Editor(typeof(OpenTexUserControlEditor), typeof(OpenTexUserControlEditor))]
			public string opacity_tex { get; set; }

			[Category("Detail")]
			[DisplayName("Mode")]
			[ItemsSource(typeof(DetailModeItemsSource))]
			[PropertyOrder((int)PropertyOrders.PO_DetailMode)]
			public string detail_mode { get; set; }
			[Category("Detail")]
			[DisplayName("Height Offset")]
			[PropertyOrder((int)PropertyOrders.PO_HeightOffset)]
			public float height_offset { get; set; }
			[Category("Detail")]
			[DisplayName("Height Scale")]
			[PropertyOrder((int)PropertyOrders.PO_HeightScale)]
			public float height_scale { get; set; }
			[Category("Detail")]
			[DisplayName("Edge Tessellation Hint")]
			[PropertyOrder((int)PropertyOrders.PO_EdgeTessHint)]
			public float edge_tess_hint { get; set; }
			[Category("Detail")]
			[DisplayName("Inside Tessellation Hint")]
			[PropertyOrder((int)PropertyOrders.PO_InsideTessHint)]
			public float inside_tess_hint { get; set; }
			[Category("Detail")]
			[DisplayName("Min Tessellation")]
			[PropertyOrder((int)PropertyOrders.PO_MinTess)]
			public float min_tess { get; set; }
			[Category("Detail")]
			[DisplayName("Max Tessellation")]
			[PropertyOrder((int)PropertyOrders.PO_MaxTess)]
			public float max_tess { get; set; }
		}

		public MainWindow()
		{
			InitializeComponent();

			DataContext = this;

			MeshItemsSource.items = new Xceed.Wpf.Toolkit.PropertyGrid.Attributes.ItemCollection();

			DetailModeItemsSource.items = new Xceed.Wpf.Toolkit.PropertyGrid.Attributes.ItemCollection();
			DetailModeItemsSource.items.Clear();
			DetailModeItemsSource.items.Add("Parallax");
			DetailModeItemsSource.items.Add("Flat Tessellation");
			DetailModeItemsSource.items.Add("Smooth Tessellation");

			properties_obj_ = new ModelPropertyTypes();
			properties_obj_.vertex_streams = new List<string>();
			properties.SelectedObject = properties_obj_;

			save.IsEnabled = false;
			save_as.IsEnabled = false;
			undo.IsEnabled = false;
			redo.IsEnabled = false;
			skinning.IsEnabled = false;
			play.IsEnabled = false;
			visualize.IsEnabled = false;
			frame_text.IsEnabled = false;
			frame_slider.IsEnabled = false;

			frame_slider.Minimum = 0;
			frame_slider.Maximum = 1;

			properties.IsEnabled = false;

			last_time_ = DateTime.Now;

			Uri iconUri = new Uri("pack://application:,,,/Images/klayge_logo.ico", UriKind.RelativeOrAbsolute);
			this.Icon = BitmapFrame.Create(iconUri);
		}

		void MainWindowLoaded(object sender, RoutedEventArgs e)
		{
			IntPtr wnd = editor_wnd.Handle;
			core_ = new KlayGE.MtlEditorCoreWrapper(wnd);

			core_.UpdateSelectEntityCallback(this.UpdateSelectEntity);

			CompositionTarget.Rendering += this.MainWindowIdle;
		}

		void MainWindowUnloaded(object sender, RoutedEventArgs e)
		{
			CompositionTarget.Rendering -= this.MainWindowIdle;
			core_.Dispose();
			core_ = null;
		}

		private void MainWindowIdle(object sender, EventArgs e)
		{
			core_.Refresh();

			if (play_)
			{
				DateTime this_time = DateTime.Now;
				if (this_time.Subtract(last_time_).TotalSeconds > 0.02)
				{
					frame_ += 0.02 * core_.ModelFrameRate();
					frame_ = frame_ % (float)core_.NumFrames();

					last_time_ = this_time;
				}

				frame_slider.Value = frame_;
			}
		}

		private void EditorWindowSizeChanged(object sender, SizeChangedEventArgs e)
		{
			editor_frame.Width = editor_bg.ActualWidth;
			editor_frame.Height = editor_bg.ActualHeight;

			core_.Resize((uint)editor_frame.Width, (uint)editor_frame.Height);
		}

		private void OpenModel(string file_name)
		{
			string ext_name = System.IO.Path.GetExtension(file_name);
			if ((ext_name != ".meshml") && (ext_name != ".model_bin"))
			{
				return;
			}

			core_.OpenModel(file_name);
			this.FileNameChanged(file_name);

			this.ClearHistroy();
			this.UpdateHistroy();

			save.IsEnabled = true;
			save_as.IsEnabled = true;
			if (core_.NumFrames() != 0)
			{
				skinning.IsEnabled = true;
				skinning.IsChecked = true;
				play.IsEnabled = true;
				frame_text.IsEnabled = true;
				frame_slider.IsEnabled = true;
				frame_slider.Maximum = core_.NumFrames() - 1;
			}
			else
			{
				skinning.IsEnabled = false;
				skinning.IsChecked = false;
				play.IsEnabled = false;
				frame_text.IsEnabled = false;
				frame_slider.IsEnabled = false;
				frame_slider.Maximum = 1;
			}
			visualize.IsEnabled = true;
			properties.IsEnabled = true;
			// Workround for.NET 4.6.1
			visualize_gallery.Command = ApplicationCommands.Print;
			visualize_gallery.Command = null;

			frame_ = 0;

			MeshItemsSource.items.Clear();
			MeshItemsSource.items.Add("");
			uint num_meshes = core_.NumMeshes();
			for (uint i = 0; i < num_meshes; ++i)
			{
				MeshItemsSource.items.Add(core_.MeshName(i));
			}

			properties_obj_.meshes = "";
			this.UpdateMeshProperties(0);

			properties.SelectedObject = null;
			properties.SelectedObject = properties_obj_;
		}

		private void OpenClick(object sender, RoutedEventArgs e)
		{
			Microsoft.Win32.OpenFileDialog dlg = new Microsoft.Win32.OpenFileDialog();

			dlg.DefaultExt = ".meshml";
			dlg.Filter = "All Model Files (*.meshml, *.model_bin)|*.meshml;*.model_bin|MeshML Files (*.meshml)|*.meshml|model_bin Files (*.model_bin)|*.model_bin|All Files|*.*";
			dlg.CheckPathExists = true;
			dlg.CheckFileExists = true;
			if (true == dlg.ShowDialog())
			{
				this.OpenModel(dlg.FileName);
			}
		}

		private void SaveClick(object sender, RoutedEventArgs e)
		{
			core_.SaveModel(opened_file_);
		}

		private void SaveAsClick(object sender, RoutedEventArgs e)
		{
			Microsoft.Win32.SaveFileDialog dlg = new Microsoft.Win32.SaveFileDialog();

			dlg.DefaultExt = ".meshml";
			dlg.Filter = "MeshML Files (*.meshml)|*.meshml|All Files|*.*";
			dlg.OverwritePrompt = true;
			if (true == dlg.ShowDialog())
			{
				core_.SaveModel(dlg.FileName);
				this.FileNameChanged(dlg.FileName);
			}
		}

		private void UndoClick(object sender, RoutedEventArgs e)
		{
			this.Undo();

			selected_mesh_id_ = core_.SelectedMesh();
			this.UpdateMeshProperties(selected_mesh_id_);
			this.UpdateHistroy();
		}

		private void RedoClick(object sender, RoutedEventArgs e)
		{
			this.Redo();

			selected_mesh_id_ = core_.SelectedMesh();
			this.UpdateMeshProperties(selected_mesh_id_);
			this.UpdateHistroy();
		}

		public bool SkinningValue
		{
			get
			{
				return skinning_;
			}
			set
			{
				skinning_ = value;
				core_.SkinningOn(skinning_ ? 1 : 0);
				play.IsEnabled = skinning_;
			}
		}

		public bool PlayValue
		{
			get
			{
				return play_;
			}
			set
			{
				play_ = value;
			}
		}

		public bool FPSCameraValue
		{
			get
			{
				return fps_camera_;
			}
			set
			{
				fps_camera_ = value;
				core_.FPSCameraOn(fps_camera_ ? 1 : 0);
			}
		}

		public bool LineModeValue
		{
			get
			{
				return line_mode_;
			}
			set
			{
				line_mode_ = value;
				core_.LineModeOn(line_mode_ ? 1 : 0);
			}
		}

		public double FrameSliderValue
		{
			get
			{
				return frame_;
			}
			set
			{
				frame_ = value;
				core_.CurrFrame((float)frame_);
				frame_text.Content = "Frame " + (int)(frame_ + 0.5f);

				this.UpdateHistroy();
			}
		}

		private void VisualizeSelectionChanged(object sender, RoutedPropertyChangedEventArgs<object> e)
		{
			if (core_ != null)
			{
				System.Windows.Controls.Ribbon.RibbonGalleryItem item = e.NewValue as System.Windows.Controls.Ribbon.RibbonGalleryItem;
				core_.Visualize(Int32.Parse((string)item.DataContext));
			}
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

			if (MouseButtons.Left == e.Button)
			{
				uint selected_mesh = core_.SelectedMesh();
				if (selected_mesh != selected_mesh_id_)
				{
					this.UpdateMeshProperties(selected_mesh);
					this.UpdateHistroy();
				}
			}
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

		private void EditorKeyPress(object sender, System.Windows.Forms.KeyPressEventArgs e)
		{
			core_.KeyPress(e.KeyChar);
		}

		private void UpdateSelectEntity(uint mesh_id)
		{
			this.ExecuteCommand(new MtlEditorCommandSelectMesh(core_, mesh_id));
		}

		private void UpdateMeshProperties(uint mesh_id)
		{
			selected_mesh_id_ = mesh_id;

			properties.SelectedObject = null;

			properties_obj_.meshes = MeshItemsSource.items[(int)mesh_id].DisplayName;

			properties_obj_.vertex_streams.Clear();

			if (mesh_id > 0)
			{
				uint num_vss = core_.NumVertexStreams(mesh_id - 1);
				for (uint stream_index = 0; stream_index < num_vss; ++stream_index)
				{
					string stream_name = "";
					uint num_usages = core_.NumVertexStreamUsages(mesh_id - 1, stream_index);
					for (uint usage_index = 0; usage_index < num_usages; ++usage_index)
					{
						uint usage = core_.VertexStreamUsage(mesh_id - 1, stream_index, usage_index);
						string usage_name;
						switch (usage >> 16)
						{
							case 0:
								usage_name = "Position";
								break;
							case 1:
								usage_name = "Normal";
								break;
							case 2:
								usage_name = "Diffuse";
								break;
							case 3:
								usage_name = "Specular";
								break;
							case 4:
								usage_name = "Blend Weight";
								break;
							case 5:
								usage_name = "Blend Index";
								break;
							case 6:
								usage_name = "TexCoord";
								break;
							case 7:
								usage_name = "Tangent";
								break;
							case 8:
							default:
								usage_name = "Binormal";
								break;
						}
						stream_name += usage_name + ' ' + (usage & 0xFFFF).ToString();
						if (usage_index != num_usages - 1)
						{
							stream_name += " | ";
						}
					}

					properties_obj_.vertex_streams.Add(stream_name);
				}

				uint mtl_id = core_.MaterialID(mesh_id);

				properties_obj_.ambient = FloatPtrToColor(core_.AmbientMaterial(mtl_id));
				properties_obj_.diffuse = FloatPtrToColor(core_.DiffuseMaterial(mtl_id));
				properties_obj_.specular = FloatPtrToColor(core_.SpecularMaterial(mtl_id));
				properties_obj_.emit = FloatPtrToColor(core_.EmitMaterial(mtl_id));
				properties_obj_.shininess = core_.ShininessMaterial(mtl_id);
				properties_obj_.opacity = core_.OpacityMaterial(mtl_id);

				properties_obj_.diffuse_tex = core_.DiffuseTexture(mtl_id);
				properties_obj_.specular_tex = core_.SpecularTexture(mtl_id);
				properties_obj_.shininess_tex = core_.ShininessTexture(mtl_id);
				properties_obj_.normal_tex = core_.NormalTexture(mtl_id);
				properties_obj_.height_tex = core_.HeightTexture(mtl_id);
				properties_obj_.emit_tex = core_.EmitTexture(mtl_id);
				properties_obj_.opacity_tex = core_.OpacityTexture(mtl_id);

				properties_obj_.detail_mode = DetailModeItemsSource.items[(int)core_.DetailMode(mtl_id)].DisplayName;
				properties_obj_.height_offset = core_.HeightOffset(mtl_id);
				properties_obj_.height_scale = core_.HeightScale(mtl_id);
				properties_obj_.edge_tess_hint = core_.EdgeTessHint(mtl_id);
				properties_obj_.inside_tess_hint = core_.InsideTessHint(mtl_id);
				properties_obj_.min_tess = core_.MinTess(mtl_id);
				properties_obj_.max_tess = core_.MaxTess(mtl_id);
			}
			else
			{
				properties_obj_.ambient = Color.FromArgb(0, 0, 0, 0);
				properties_obj_.diffuse = Color.FromArgb(0, 0, 0, 0);
				properties_obj_.specular = Color.FromArgb(0, 0, 0, 0);
				properties_obj_.emit = Color.FromArgb(0, 0, 0, 0);
				properties_obj_.shininess = 0;
				properties_obj_.opacity = 1;

				properties_obj_.diffuse_tex = "";
				properties_obj_.specular_tex = "";
				properties_obj_.shininess_tex = "";
				properties_obj_.normal_tex = "";
				properties_obj_.height_tex = "";
				properties_obj_.emit_tex = "";
				properties_obj_.opacity_tex = "";

				properties_obj_.detail_mode = "Parallax";
				properties_obj_.height_offset = -0.5f;
				properties_obj_.height_scale = 0.06f;
				properties_obj_.edge_tess_hint = 5;
				properties_obj_.inside_tess_hint = 5;
				properties_obj_.min_tess = 1;
				properties_obj_.max_tess = 9;
			}

			properties.SelectedObject = properties_obj_;
		}

		private void PropertyGridValueChanged(object sender, Xceed.Wpf.Toolkit.PropertyGrid.PropertyValueChangedEventArgs e)
		{
			Xceed.Wpf.Toolkit.PropertyGrid.PropertyItem item = e.OriginalSource as Xceed.Wpf.Toolkit.PropertyGrid.PropertyItem;
			switch ((PropertyOrders)item.PropertyOrder)
			{
				case PropertyOrders.PO_Meshes:
					{
						uint mesh_id = 0;
						for (; mesh_id < MeshItemsSource.items.Count; ++ mesh_id)
						{
							if (MeshItemsSource.items[(int)mesh_id].DisplayName == (e.NewValue as string))
							{
								break;
							}
						}

						if (core_.SelectedMesh() != mesh_id)
						{
							this.ExecuteCommand(new MtlEditorCommandSelectMesh(core_, mesh_id));
							this.UpdateMeshProperties(mesh_id);
						}
					}
					break;

				case PropertyOrders.PO_Ambient:
					if (selected_mesh_id_ > 0)
					{
						uint mtl_id = core_.MaterialID(selected_mesh_id_);
						float[] ambient = ColorToFloatPtr(properties_obj_.ambient);
						float[] old_ambient = core_.AmbientMaterial(mtl_id);
						if (!this.FloatEqual(old_ambient[0], ambient[0]) || !this.FloatEqual(old_ambient[1], ambient[1])
							|| !this.FloatEqual(old_ambient[2], ambient[2]))
						{
							this.ExecuteCommand(new MtlEditorCommandSetAmbientMaterial(core_, mtl_id, ambient));
						}
					}
					break;

				case PropertyOrders.PO_Diffuse:
					if (selected_mesh_id_ > 0)
					{
						uint mtl_id = core_.MaterialID(selected_mesh_id_);
						float[] diffuse = ColorToFloatPtr(properties_obj_.diffuse);
						float[] old_diffuse = core_.DiffuseMaterial(mtl_id);
						if (!this.FloatEqual(old_diffuse[0], diffuse[0]) || !this.FloatEqual(old_diffuse[1], diffuse[1])
							|| !this.FloatEqual(old_diffuse[2], diffuse[2]))
						{
							this.ExecuteCommand(new MtlEditorCommandSetDiffuseMaterial(core_, mtl_id, diffuse));
						}
					}
					break;

				case PropertyOrders.PO_Specular:
					if (selected_mesh_id_ > 0)
					{
						uint mtl_id = core_.MaterialID(selected_mesh_id_);
						float[] specular = ColorToFloatPtr(properties_obj_.specular);
						float[] old_specular = core_.SpecularMaterial(mtl_id);
						if (!this.FloatEqual(old_specular[0], specular[0]) || !this.FloatEqual(old_specular[1], specular[1])
							|| !this.FloatEqual(old_specular[2], specular[2]))
						{
							this.ExecuteCommand(new MtlEditorCommandSetSpecularMaterial(core_, mtl_id, specular));
						}
					}
					break;

				case PropertyOrders.PO_Shininess:
					if (selected_mesh_id_ > 0)
					{
						uint mtl_id = core_.MaterialID(selected_mesh_id_);
						if (core_.ShininessMaterial(mtl_id) != properties_obj_.shininess)
						{
							this.ExecuteCommand(new MtlEditorCommandSetShininessMaterial(core_, mtl_id, properties_obj_.shininess));
						}
					}
					break;

				case PropertyOrders.PO_Emit:
					if (selected_mesh_id_ > 0)
					{
						uint mtl_id = core_.MaterialID(selected_mesh_id_);
						float[] emit = ColorToFloatPtr(properties_obj_.emit);
						float[] old_emit = core_.EmitMaterial(mtl_id);
						if (!this.FloatEqual(old_emit[0], emit[0]) || !this.FloatEqual(old_emit[1], emit[1])
							|| !this.FloatEqual(old_emit[2], emit[2]))
						{
							this.ExecuteCommand(new MtlEditorCommandSetEmitMaterial(core_, mtl_id, emit));
						}
					}
					break;

				case PropertyOrders.PO_Opacity:
					if (selected_mesh_id_ > 0)
					{
						uint mtl_id = core_.MaterialID(selected_mesh_id_);
						if (!this.FloatEqual(core_.OpacityMaterial(mtl_id), properties_obj_.opacity))
						{
							this.ExecuteCommand(new MtlEditorCommandSetOpacityMaterial(core_, mtl_id, properties_obj_.opacity));
						}
					}
					break;

				case PropertyOrders.PO_DiffuseTex:
					if (selected_mesh_id_ > 0)
					{
						uint mtl_id = core_.MaterialID(selected_mesh_id_);
						string diffuse_tex = RelativePath(properties_obj_.diffuse_tex);
						if (core_.DiffuseTexture(mtl_id) != diffuse_tex)
						{
							this.ExecuteCommand(new MtlEditorCommandSetDiffuseTexture(core_, mtl_id, diffuse_tex));
						}
					}
					break;

				case PropertyOrders.PO_SpecularTex:
					if (selected_mesh_id_ > 0)
					{
						uint mtl_id = core_.MaterialID(selected_mesh_id_);
						string specular_tex = RelativePath(properties_obj_.specular_tex);
						if (core_.SpecularTexture(mtl_id) != specular_tex)
						{
							this.ExecuteCommand(new MtlEditorCommandSetSpecularTexture(core_, mtl_id, specular_tex));
						}
					}
					break;

				case PropertyOrders.PO_ShininessTex:
					if (selected_mesh_id_ > 0)
					{
						uint mtl_id = core_.MaterialID(selected_mesh_id_);
						string shininess_tex = RelativePath(properties_obj_.shininess_tex);
						if (core_.ShininessTexture(mtl_id) != shininess_tex)
						{
							this.ExecuteCommand(new MtlEditorCommandSetShininessTexture(core_, mtl_id, shininess_tex));
						}
					}
					break;

				case PropertyOrders.PO_NormalTex:
					if (selected_mesh_id_ > 0)
					{
						uint mtl_id = core_.MaterialID(selected_mesh_id_);
						string normal_tex = RelativePath(properties_obj_.normal_tex);
						if (core_.NormalTexture(mtl_id) != normal_tex)
						{
							this.ExecuteCommand(new MtlEditorCommandSetNormalTexture(core_, mtl_id, normal_tex));
						}
					}
					break;

				case PropertyOrders.PO_HeightTex:
					if (selected_mesh_id_ > 0)
					{
						uint mtl_id = core_.MaterialID(selected_mesh_id_);
						string height_tex = RelativePath(properties_obj_.height_tex);
						if (core_.HeightTexture(mtl_id) != height_tex)
						{
							this.ExecuteCommand(new MtlEditorCommandSetHeightTexture(core_, mtl_id, height_tex));
						}
					}
					break;

				case PropertyOrders.PO_EmitTex:
					if (selected_mesh_id_ > 0)
					{
						uint mtl_id = core_.MaterialID(selected_mesh_id_);
						string emit_tex = RelativePath(properties_obj_.emit_tex);
						if (core_.EmitTexture(mtl_id) != emit_tex)
						{
							this.ExecuteCommand(new MtlEditorCommandSetEmitTexture(core_, mtl_id, emit_tex));
						}
					}
					break;

				case PropertyOrders.PO_OpacityTex:
					if (selected_mesh_id_ > 0)
					{
						uint mtl_id = core_.MaterialID(selected_mesh_id_);
						string opacity_tex = RelativePath(properties_obj_.opacity_tex);
						if (core_.OpacityTexture(mtl_id) != opacity_tex)
						{
							this.ExecuteCommand(new MtlEditorCommandSetOpacityTexture(core_, mtl_id, opacity_tex));
						}
					}
					break;

				case PropertyOrders.PO_DetailMode:
					if (selected_mesh_id_ > 0)
					{
						uint mode = 0;
						for (; mode < DetailModeItemsSource.items.Count; ++ mode)
						{
							if (DetailModeItemsSource.items[(int)mode].DisplayName == (e.NewValue as string))
							{
								break;
							}
						}

						uint mtl_id = core_.MaterialID(selected_mesh_id_);
						if (core_.DetailMode(mtl_id) != mode)
						{
							this.ExecuteCommand(new MtlEditorCommandSetDetailMode(core_, mtl_id, mode));
						}
					}
					break;

				case PropertyOrders.PO_HeightOffset:
					if (selected_mesh_id_ > 0)
					{
						uint mtl_id = core_.MaterialID(selected_mesh_id_);
						if (!this.FloatEqual(core_.HeightOffset(mtl_id), properties_obj_.height_offset))
						{
							this.ExecuteCommand(new MtlEditorCommandSetHeightOffset(core_, mtl_id, properties_obj_.height_offset));
						}
					}
					break;

				case PropertyOrders.PO_HeightScale:
					if (selected_mesh_id_ > 0)
					{
						uint mtl_id = core_.MaterialID(selected_mesh_id_);
						if (!this.FloatEqual(core_.HeightScale(mtl_id), properties_obj_.height_scale))
						{
							this.ExecuteCommand(new MtlEditorCommandSetHeightScale(core_, mtl_id, properties_obj_.height_scale));
						}
					}
					break;

				case PropertyOrders.PO_EdgeTessHint:
					if (selected_mesh_id_ > 0)
					{
						uint mtl_id = core_.MaterialID(selected_mesh_id_);
						if (!this.FloatEqual(core_.EdgeTessHint(mtl_id), properties_obj_.edge_tess_hint))
						{
							this.ExecuteCommand(new MtlEditorCommandSetEdgeTessHint(core_, mtl_id, properties_obj_.edge_tess_hint));
						}
					}
					break;

				case PropertyOrders.PO_InsideTessHint:
					if (selected_mesh_id_ > 0)
					{
						uint mtl_id = core_.MaterialID(selected_mesh_id_);
						if (!this.FloatEqual(core_.InsideTessHint(mtl_id), properties_obj_.inside_tess_hint))
						{
							this.ExecuteCommand(new MtlEditorCommandSetInsideTessHint(core_, mtl_id, properties_obj_.inside_tess_hint));
						}
					}
					break;

				case PropertyOrders.PO_MinTess:
					if (selected_mesh_id_ > 0)
					{
						uint mtl_id = core_.MaterialID(selected_mesh_id_);
						if (!this.FloatEqual(core_.MinTess(mtl_id), properties_obj_.min_tess))
						{
							this.ExecuteCommand(new MtlEditorCommandSetMinTess(core_, mtl_id, properties_obj_.min_tess));
						}
					}
					break;

				case PropertyOrders.PO_MaxTess:
					if (selected_mesh_id_ > 0)
					{
						uint mtl_id = core_.MaterialID(selected_mesh_id_);
						if (!this.FloatEqual(core_.MaxTess(mtl_id), properties_obj_.max_tess))
						{
							this.ExecuteCommand(new MtlEditorCommandSetMaxTess(core_, mtl_id, properties_obj_.max_tess));
						}
					}
					break;

				default:
					break;
			}
		}

		private void FileNameChanged(string name)
		{
			opened_file_ = name;
			int dot_offset = opened_file_.LastIndexOf('.');
			if (".model_bin" == opened_file_.Substring(dot_offset))
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
			else
			{
				Uri uri_meshml = new Uri(opened_file_);
				Uri uri_tex = new Uri(name);
				Uri relative_uri = uri_meshml.MakeRelativeUri(uri_tex);
				return relative_uri.ToString();
			}
		}

		private void UpdateHistroy()
		{
			undo.IsEnabled = (end_command_index_ > 0);
			redo.IsEnabled = (end_command_index_ < command_history_.Count);
		}

		string HistroyCmdName(int index)
		{
			return command_history_[index].Name();
		}

		private object ExecuteCommand(MtlEditorCommand cmd)
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
				this.OpenModel(files[0]);
			}
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

		private Color FloatPtrToColor(float[] clr)
		{
			float[] temp = new float[3];
			for (int i = 0; i < 3; ++i)
			{
				temp[i] = LinearToSRGB(clr[i]);
			}
			return Color.FromArgb(255,
				(byte)(Math.Max(Math.Min((int)(temp[0] * 255 + 0.5f), 255), 0)),
				(byte)(Math.Max(Math.Min((int)(temp[1] * 255 + 0.5f), 255), 0)),
				(byte)(Math.Max(Math.Min((int)(temp[2] * 255 + 0.5f), 255), 0)));
		}

		private float[] ColorToFloatPtr(Color clr)
		{
			float[] ret = new float[3];
			ret[0] = clr.R / 255.0f;
			ret[1] = clr.G / 255.0f;
			ret[2] = clr.B / 255.0f;
			for (int i = 0; i < 3; ++i)
			{
				ret[i] = SRGBToLinear(ret[i]);
			}
			return ret;
		}

		private bool FloatEqual(float a, float b)
		{
			return Math.Abs(a - b) < 1e-6f;
		}

		private KlayGE.MtlEditorCoreWrapper core_;
		private DateTime last_time_;
		private double frame_;
		private ModelPropertyTypes properties_obj_;
		private uint selected_mesh_id_ = 0;
		private string opened_file_ = "";

		private bool skinning_ = false;
		private bool fps_camera_ = false;
		private bool line_mode_ = false;
		private bool play_ = false;

		private List<MtlEditorCommand> command_history_ = new List<MtlEditorCommand>();
		private int end_command_index_ = 0;
	}

	public class MeshItemsSource : IItemsSource
	{
		static public Xceed.Wpf.Toolkit.PropertyGrid.Attributes.ItemCollection items;

		public Xceed.Wpf.Toolkit.PropertyGrid.Attributes.ItemCollection GetValues()
		{
			return items;
		}
	}

	public class DetailModeItemsSource : IItemsSource
	{
		static public Xceed.Wpf.Toolkit.PropertyGrid.Attributes.ItemCollection items;

		public Xceed.Wpf.Toolkit.PropertyGrid.Attributes.ItemCollection GetValues()
		{
			return items;
		}
	}
}
