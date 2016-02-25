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

namespace MeshMLViewer
{
	/// <summary>
	/// Interaction logic for MainWindow.xaml
	/// </summary>
	public partial class MainWindow : RibbonWindow
	{
		const int MESHES_ORDER = 0;
		const int VERTEX_STREAMS_ORDER = 1;
		const int AMBIENT_ORDER = 2;
		const int DIFFUSE_ORDER = 3;
		const int SPECULAR_ORDER = 4;
		const int SHININESS_ORDER = 5;
		const int EMIT_ORDER = 6;
		const int OPACITY_ORDER = 7;
		const int DIFFUSE_TEX_ORDER = 8;
		const int SPECULAR_TEX_ORDER = 9;
		const int SHININESS_TEX_ORDER = 10;
		const int NORMAL_TEX_ORDER = 11;
		const int HEIGHT_TEX_ORDER = 12;
		const int EMIT_TEX_ORDER = 13;
		const int OPACITY_TEX_ORDER = 14;

		[CategoryOrder("Meshes", 0)]
		[CategoryOrder("Vertex Streams", 1)]
		[CategoryOrder("Material", 2)]
		[CategoryOrder("Textures", 3)]
		public class ModelPropertyTypes
		{
			[Category("Meshes")]
			[DisplayName("Meshes")]
			[ItemsSource(typeof(MeshItemsSource))]
			[PropertyOrder(MESHES_ORDER)]
			public string meshes { get; set; }

			[Category("Vertex Streams")]
			[DisplayName("Vertex Streams")]
			[PropertyOrder(VERTEX_STREAMS_ORDER)]
			public List<string> vertex_streams { get; set; }

			[Category("Material")]
			[DisplayName("Ambient")]
			[PropertyOrder(AMBIENT_ORDER)]
			public Color ambient { get; set; }
			[Category("Material")]
			[DisplayName("Diffuse")]
			[PropertyOrder(DIFFUSE_ORDER)]
			public Color diffuse { get; set; }
			[Category("Material")]
			[DisplayName("Specular")]
			[PropertyOrder(SPECULAR_ORDER)]
			public Color specular { get; set; }
			[Category("Material")]
			[DisplayName("Shininess")]
			[PropertyOrder(SHININESS_ORDER)]
			public float shininess { get; set; }
			[Category("Material")]
			[DisplayName("Emit")]
			[PropertyOrder(EMIT_ORDER)]
			public Color emit { get; set; }
			[Category("Material")]
			[DisplayName("Opacity")]
			[PropertyOrder(OPACITY_ORDER)]
			public float opacity { get; set; }

			[Category("Textures")]
			[DisplayName("Diffuse")]
			[PropertyOrder(DIFFUSE_TEX_ORDER)]
			public string diffuse_tex { get; set; }
			[Category("Textures")]
			[DisplayName("Specular")]
			[PropertyOrder(SPECULAR_TEX_ORDER)]
			public string specular_tex { get; set; }
			[Category("Textures")]
			[DisplayName("Shininess")]
			[PropertyOrder(SHININESS_TEX_ORDER)]
			public string shininess_tex { get; set; }
			[Category("Textures")]
			[DisplayName("Normal")]
			[PropertyOrder(NORMAL_TEX_ORDER)]
			public string normal_tex { get; set; }
			[Category("Textures")]
			[DisplayName("Height")]
			[PropertyOrder(HEIGHT_TEX_ORDER)]
			public string height_tex { get; set; }
			[Category("Textures")]
			[DisplayName("Emit")]
			[PropertyOrder(EMIT_TEX_ORDER)]
			public string emit_tex { get; set; }
			[Category("Textures")]
			[DisplayName("Opacity")]
			[PropertyOrder(OPACITY_TEX_ORDER)]
			public string opacity_tex { get; set; }
		}

		public MainWindow()
		{
			InitializeComponent();

			DataContext = this;

			MeshItemsSource.items = new Xceed.Wpf.Toolkit.PropertyGrid.Attributes.ItemCollection();

			properties_obj_ = new ModelPropertyTypes();
			properties_obj_.vertex_streams = new List<string>();
			properties.SelectedObject = properties_obj_;

			skinning.IsEnabled = false;
			play.IsEnabled = false;
			smooth.IsEnabled = false;
			line_mode.IsEnabled = false;
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
			IntPtr wnd = viewer_wnd.Handle;
			core_ = new KlayGE.MeshMLViewerCoreWrapper(wnd);

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

		private void ViewerWindowSizeChanged(object sender, SizeChangedEventArgs e)
		{
			viewer_frame.Width = viewer_bg.ActualWidth;
			viewer_frame.Height = viewer_bg.ActualHeight;

			core_.Resize((uint)viewer_frame.Width, (uint)viewer_frame.Height);
		}

		private void OpenModel(string file_name)
		{
			string ext_name = System.IO.Path.GetExtension(file_name);
			if ((ext_name != ".meshml") && (ext_name != ".model_bin"))
			{
				return;
			}

			core_.OpenModel(file_name);

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
			smooth.IsEnabled = true;
			line_mode.IsEnabled = true;
			visualize.IsEnabled = true;
			properties.IsEnabled = true;
			// Workround for.NET 4.6.1
			visualize_gallery.Command = ApplicationCommands.Print;
			visualize_gallery.Command = null;

			frame_ = 0;

			properties.SelectedObject = null;

			MeshItemsSource.items.Clear();
			MeshItemsSource.items.Add("");
			uint num_meshes = core_.NumMeshes();
			for (uint i = 0; i < num_meshes; ++i)
			{
				MeshItemsSource.items.Add(core_.MeshName(i));
			}

			properties_obj_.meshes = "";
			this.UpdateMeshProperties(0);

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
			Microsoft.Win32.SaveFileDialog dlg = new Microsoft.Win32.SaveFileDialog();

			dlg.DefaultExt = ".meshml";
			dlg.Filter = "MeshML Files (*.meshml)|*.meshml|All Files|*.*";
			dlg.OverwritePrompt = true;
			if (true == dlg.ShowDialog())
			{
				core_.SaveModel(dlg.FileName);
			}
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

		public bool SmoothMeshValue
		{
			get
			{
				return smooth_mesh_;
			}
			set
			{
				smooth_mesh_ = value;
				core_.SmoothMeshOn(smooth_mesh_ ? 1 : 0);
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

		private void ViewerMouseDown(object sender, System.Windows.Forms.MouseEventArgs e)
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
		private void ViewerMouseUp(object sender, System.Windows.Forms.MouseEventArgs e)
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
				}
			}
		}
		private void ViewerMouseMove(object sender, System.Windows.Forms.MouseEventArgs e)
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
		private void ViewerKeyPress(object sender, System.Windows.Forms.KeyPressEventArgs e)
		{
			core_.KeyPress(e.KeyChar);
		}

		private void UpdateMeshProperties(uint mesh_id)
		{
			selected_mesh_id_ = mesh_id;
			core_.SelectMesh(mesh_id);

			properties.SelectedObject = null;

			properties_obj_.meshes = MeshItemsSource.items[(int)mesh_id].DisplayName;

			properties_obj_.vertex_streams.Clear();

			if (mesh_id > 0)
			{
				uint num_vss = core_.NumVertexStreams(mesh_id - 1);
				for (uint stream_index = 0; stream_index < num_vss; ++ stream_index)
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

				properties_obj_.ambient = core_.AmbientMaterial(mtl_id);
				properties_obj_.diffuse = core_.DiffuseMaterial(mtl_id);
				properties_obj_.specular = core_.SpecularMaterial(mtl_id);
				properties_obj_.emit = core_.EmitMaterial(mtl_id);
				properties_obj_.shininess = core_.ShininessMaterial(mtl_id);
				properties_obj_.opacity = core_.OpacityMaterial(mtl_id);

				properties_obj_.diffuse_tex = core_.DiffuseTexture(mtl_id);
				properties_obj_.specular_tex = core_.SpecularTexture(mtl_id);
				properties_obj_.shininess_tex = core_.ShininessTexture(mtl_id);
				properties_obj_.normal_tex = core_.NormalTexture(mtl_id);
				properties_obj_.height_tex = core_.HeightTexture(mtl_id);
				properties_obj_.emit_tex = core_.EmitTexture(mtl_id);
				properties_obj_.opacity_tex = core_.OpacityTexture(mtl_id);
			}
			else
			{
				properties_obj_.ambient = Color.FromArgb(0, 0, 0, 0);
				properties_obj_.diffuse = Color.FromArgb(0, 0, 0, 0);
				properties_obj_.specular = Color.FromArgb(0, 0, 0, 0);
				properties_obj_.emit = Color.FromArgb(0, 0, 0, 0);
				properties_obj_.shininess = 0;
				properties_obj_.opacity = 0;

				properties_obj_.diffuse_tex = "";
				properties_obj_.specular_tex = "";
				properties_obj_.shininess_tex = "";
				properties_obj_.normal_tex = "";
				properties_obj_.height_tex = "";
				properties_obj_.emit_tex = "";
				properties_obj_.opacity_tex = "";
			}

			properties.SelectedObject = properties_obj_;
		}

		private void PropertyGridValueChanged(object sender, Xceed.Wpf.Toolkit.PropertyGrid.PropertyValueChangedEventArgs e)
		{
			Xceed.Wpf.Toolkit.PropertyGrid.PropertyItem item = e.OriginalSource as Xceed.Wpf.Toolkit.PropertyGrid.PropertyItem;
			if (MESHES_ORDER == item.PropertyOrder)
			{
				uint mesh_id = 0;
				for (; mesh_id < MeshItemsSource.items.Count; ++ mesh_id)
				{
					if (MeshItemsSource.items[(int)mesh_id].DisplayName == (e.NewValue as string))
					{
						break;
					}
				}

				this.UpdateMeshProperties(mesh_id);
			}
		}

		private void ViewerBGDragEnter(object sender, System.Windows.DragEventArgs e)
		{
			if (!e.Data.GetDataPresent(System.Windows.DataFormats.FileDrop) || (sender == e.Source))
			{
				e.Effects = System.Windows.DragDropEffects.None;
			}
		}

		private void ViewerBGDrop(object sender, System.Windows.DragEventArgs e)
		{
			if (e.Data.GetDataPresent(System.Windows.DataFormats.FileDrop))
			{
				string[] files = (string[])e.Data.GetData(System.Windows.DataFormats.FileDrop);
				this.OpenModel(files[0]);
			}
		}

		private KlayGE.MeshMLViewerCoreWrapper core_;
		private DateTime last_time_;
		private double frame_;
		private ModelPropertyTypes properties_obj_;
		private uint selected_mesh_id_ = 0;

		private bool skinning_ = false;
		private bool smooth_mesh_ = false;
		private bool fps_camera_ = false;
		private bool line_mode_ = false;
		private bool play_ = false;
	}

	public class MeshItemsSource : IItemsSource
	{
		static public Xceed.Wpf.Toolkit.PropertyGrid.Attributes.ItemCollection items;

		public Xceed.Wpf.Toolkit.PropertyGrid.Attributes.ItemCollection GetValues()
		{
			return items;
		}
	}
}
