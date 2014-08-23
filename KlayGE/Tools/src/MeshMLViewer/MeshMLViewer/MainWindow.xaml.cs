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
using System.Runtime.InteropServices;
using System.Windows.Forms;

namespace MeshMLViewer
{
	public sealed class MeshMLViewerCore
	{
#if DEBUG
		const string CORE_NAME = "MeshMLViewerCore_d.dll";
#else
		const string CORE_NAME = "MeshMLViewerCore.dll";
#endif
		[DllImport(CORE_NAME)]
		public static extern IntPtr Create(IntPtr native_wnd);
		[DllImport(CORE_NAME)]
		public static extern void Destroy(IntPtr core);
		[DllImport(CORE_NAME)]
		public static extern void Refresh(IntPtr core);
		[DllImport(CORE_NAME)]
		public static extern void Resize(IntPtr core, uint width, uint height);
		[DllImport(CORE_NAME)]
		public static extern void OpenModel(IntPtr core, string name);
		[DllImport(CORE_NAME)]
		public static extern void SaveModel(IntPtr core, string name);
		[DllImport(CORE_NAME)]
		public static extern uint NumFrames(IntPtr core);
		[DllImport(CORE_NAME)]
		public static extern void CurrFrame(IntPtr core, float frame);
		[DllImport(CORE_NAME)]
		public static extern float ModelFrameRate(IntPtr core);
		[DllImport(CORE_NAME)]
		public static extern void SkinningOn(IntPtr core, int on);
		[DllImport(CORE_NAME)]
		public static extern void SmoothMeshOn(IntPtr core, int on);
		[DllImport(CORE_NAME)]
		public static extern void FPSCameraOn(IntPtr core, int on);
		[DllImport(CORE_NAME)]
		public static extern void LineModeOn(IntPtr core, int on);
		[DllImport(CORE_NAME)]
		public static extern void Visualize(IntPtr core, int index);
		[DllImport(CORE_NAME)]
		public static extern void MouseMove(IntPtr core, int x, int y, uint button);
		[DllImport(CORE_NAME)]
		public static extern void MouseUp(IntPtr core, int x, int y, uint button);
		[DllImport(CORE_NAME)]
		public static extern void MouseDown(IntPtr core, int x, int y, uint button);
		[DllImport(CORE_NAME)]
		public static extern void KeyPress(IntPtr core, int key);
		[DllImport(CORE_NAME)]
		public static extern uint NumMeshes(IntPtr core);
		[DllImport(CORE_NAME)]
		public static extern IntPtr MeshName(IntPtr core, uint index);
		[DllImport(CORE_NAME)]
		public static extern uint NumVertexStreams(IntPtr core, uint mesh_index);
		[DllImport(CORE_NAME)]
		public static extern uint NumVertexStreamUsages(IntPtr core, uint mesh_index, uint stream_index);
		[DllImport(CORE_NAME)]
		public static extern uint VertexStreamUsage(IntPtr core, uint mesh_index, uint stream_index,
			uint usage_index);
		[DllImport(CORE_NAME)]
		public static extern uint MaterialID(IntPtr core, uint mesh_index);
		[DllImport(CORE_NAME)]
		public static extern IntPtr AmbientMaterial(IntPtr core, uint material_index);
		[DllImport(CORE_NAME)]
		public static extern IntPtr DiffuseMaterial(IntPtr core, uint material_index);
		[DllImport(CORE_NAME)]
		public static extern IntPtr SpecularMaterial(IntPtr core, uint material_index);
		[DllImport(CORE_NAME)]
		public static extern float ShininessMaterial(IntPtr core, uint material_index);
		[DllImport(CORE_NAME)]
		public static extern IntPtr EmitMaterial(IntPtr core, uint material_index);
		[DllImport(CORE_NAME)]
		public static extern float OpacityMaterial(IntPtr core, uint material_index);
		[DllImport(CORE_NAME)]
		public static extern IntPtr DiffuseTexture(IntPtr core, uint material_index);
		[DllImport(CORE_NAME)]
		public static extern IntPtr SpecularTexture(IntPtr core, uint material_index);
		[DllImport(CORE_NAME)]
		public static extern IntPtr ShininessTexture(IntPtr core, uint material_index);
		[DllImport(CORE_NAME)]
		public static extern IntPtr BumpTexture(IntPtr core, uint material_index);
		[DllImport(CORE_NAME)]
		public static extern IntPtr HeightTexture(IntPtr core, uint material_index);
		[DllImport(CORE_NAME)]
		public static extern IntPtr EmitTexture(IntPtr core, uint material_index);
		[DllImport(CORE_NAME)]
		public static extern IntPtr OpacityTexture(IntPtr core, uint material_index);
		[DllImport(CORE_NAME)]
		public static extern uint SelectedMesh(IntPtr core);
		[DllImport(CORE_NAME)]
		public static extern void SelectMesh(IntPtr core, uint mesh_index);
	}

	/// <summary>
	/// Interaction logic for MainWindow.xaml
	/// </summary>
	public partial class MainWindow : RibbonWindow
	{
		private IntPtr core_;
		private DateTime last_time_;
		private double frame_;
		private ModelPropertyTypes properties_obj_;
		private uint selected_mesh_index_;

		public MainWindow()
		{
			InitializeComponent();

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

			selected_mesh_index_ = 0;
		}

		void MainWindowLoaded(object sender, RoutedEventArgs e)
		{
			IntPtr wnd = viewer_wnd.Handle;
			core_ = MeshMLViewerCore.Create(wnd);

			CompositionTarget.Rendering += this.MainWindowIdle;
		}
		void MainWindowUnloaded(object sender, RoutedEventArgs e)
		{
			CompositionTarget.Rendering -= this.MainWindowIdle;
			MeshMLViewerCore.Destroy(core_);
		}

		private void MainWindowIdle(object sender, EventArgs e)
		{
			MeshMLViewerCore.Refresh(core_);

			if (true == play.IsChecked)
			{
				DateTime this_time = DateTime.Now;
				if (this_time.Subtract(last_time_).TotalSeconds > 0.02)
				{
					frame_ += 0.02 * MeshMLViewerCore.ModelFrameRate(core_);
					frame_ = frame_ % (float)MeshMLViewerCore.NumFrames(core_);

					last_time_ = this_time;
				}

				frame_slider.Value = (int)(frame_ * 10 + 0.5f);
			}
		}
		private void ViewerWindowSizeChanged(object sender, SizeChangedEventArgs e)
		{
			viewer_frame.Width = viewer_bg.ActualWidth;
			viewer_frame.Height = viewer_bg.ActualHeight;

			MeshMLViewerCore.Resize(core_, (uint)viewer_frame.Width, (uint)viewer_frame.Height);
		}

		[CategoryOrder("Meshes", 0)]
		[CategoryOrder("Vertex Streams", 1)]
		[CategoryOrder("Material", 2)]
		[CategoryOrder("Textures", 3)]
		public class ModelPropertyTypes
		{
			[Category("Meshes")]
			[DisplayName("Meshes")]
			[ItemsSource(typeof(MeshItemsSource))]
			public string meshes { get; set; }

			[Category("Vertex Streams")]
			[DisplayName("Vertex Streams")]
			public List<string> vertex_streams { get; set; }

			[Category("Material")]
			[DisplayName("Ambient")]
			public Color ambient { get; set; }
			[Category("Material")]
			[DisplayName("Diffuse")]
			public Color diffuse { get; set; }
			[Category("Material")]
			[DisplayName("Specular")]
			public Color specular { get; set; }
			[Category("Material")]
			[DisplayName("Shininess")]
			public float shininess { get; set; }
			[Category("Material")]
			[DisplayName("Emit")]
			public Color emit { get; set; }
			[Category("Material")]
			[DisplayName("Opacity")]
			public float opacity { get; set; }

			[Category("Textures")]
			[DisplayName("Diffuse")]
			public string diffuse_tex { get; set; }
			[Category("Textures")]
			[DisplayName("Specular")]
			public string specular_tex { get; set; }
			[Category("Textures")]
			[DisplayName("Shininess")]
			public string shininess_tex { get; set; }
			[Category("Textures")]
			[DisplayName("Bump")]
			public string bump_tex { get; set; }
			[Category("Textures")]
			[DisplayName("Height")]
			public string height_tex { get; set; }
			[Category("Textures")]
			[DisplayName("Emit")]
			public string emit_tex { get; set; }
			[Category("Textures")]
			[DisplayName("Opacity")]
			public string opacity_tex { get; set; }
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
				MeshMLViewerCore.OpenModel(core_, dlg.FileName);

				if (MeshMLViewerCore.NumFrames(core_) != 0)
				{
					skinning.IsEnabled = true;
					skinning.IsChecked = true;
					play.IsEnabled = true;
					frame_text.IsEnabled = true;
					frame_slider.IsEnabled = true;
					frame_slider.Maximum = MeshMLViewerCore.NumFrames(core_) * 10 - 1;
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

				frame_ = 0;

				properties.SelectedObject = null;

				MeshItemsSource.items.Clear();
				MeshItemsSource.items.Add("");
				uint num_meshes = MeshMLViewerCore.NumMeshes(core_);
				for (uint i = 0; i < num_meshes; ++ i)
				{
					MeshItemsSource.items.Add(Marshal.PtrToStringUni(MeshMLViewerCore.MeshName(core_, i)));
				}

				properties_obj_.meshes = "";
				this.UpdateMeshProperties(0);

				properties.SelectedObject = properties_obj_;
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
				MeshMLViewerCore.SaveModel(core_, dlg.FileName);
			}
		}

		private void SkinningChecked(object sender, RoutedEventArgs e)
		{
			MeshMLViewerCore.SkinningOn(core_, 1);
			play.IsEnabled = true;
		}
		private void SkinningUnchecked(object sender, RoutedEventArgs e)
		{
			MeshMLViewerCore.SkinningOn(core_, 0);
			play.IsEnabled = false;
		}

		private void SmoothMeshChecked(object sender, RoutedEventArgs e)
		{
			MeshMLViewerCore.SmoothMeshOn(core_, 1);
		}
		private void SmoothMeshUnchecked(object sender, RoutedEventArgs e)
		{
			MeshMLViewerCore.SmoothMeshOn(core_, 0);
		}

		private void FPSCameraChecked(object sender, RoutedEventArgs e)
		{
			MeshMLViewerCore.FPSCameraOn(core_, 1);
		}
		private void FPSCameraUnchecked(object sender, RoutedEventArgs e)
		{
			MeshMLViewerCore.FPSCameraOn(core_, 0);
		}

		private void LineModeChecked(object sender, RoutedEventArgs e)
		{
			MeshMLViewerCore.LineModeOn(core_, 1);
		}
		private void LineModeUnchecked(object sender, RoutedEventArgs e)
		{
			MeshMLViewerCore.LineModeOn(core_, 0);
		}

		private void FrameSliderValueChanged(object sender, RoutedPropertyChangedEventArgs<double> e)
		{
			frame_ = frame_slider.Value * 0.1;
			MeshMLViewerCore.CurrFrame(core_, (float)frame_);
			frame_text.Content = "Frame " + (int)(frame_ + 0.5f);
		}

		private void VisualizeSelectionChanged(object sender, RoutedPropertyChangedEventArgs<object> e)
		{
			if (core_.ToInt64() != 0)
			{
				System.Windows.Controls.Ribbon.RibbonGalleryItem item = e.NewValue as System.Windows.Controls.Ribbon.RibbonGalleryItem;
				MeshMLViewerCore.Visualize(core_, Int32.Parse((string)item.DataContext));
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
			MeshMLViewerCore.MouseDown(core_, e.X, e.Y, buttons);
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
			MeshMLViewerCore.MouseUp(core_, e.X, e.Y, buttons);

			if (MouseButtons.Left == e.Button)
			{
				uint selected_mesh = MeshMLViewerCore.SelectedMesh(core_);
				if (selected_mesh != selected_mesh_index_)
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
			MeshMLViewerCore.MouseMove(core_, e.X, e.Y, buttons);
		}
		private void ViewerKeyPress(object sender, System.Windows.Forms.KeyPressEventArgs e)
		{
			MeshMLViewerCore.KeyPress(core_, e.KeyChar);
		}

		private void UpdateMeshProperties(uint mesh_index)
		{
			selected_mesh_index_ = mesh_index;
			MeshMLViewerCore.SelectMesh(core_, mesh_index);

			properties.SelectedObject = null;

			properties_obj_.meshes = MeshItemsSource.items[(int)mesh_index].DisplayName;

			properties_obj_.vertex_streams.Clear();

			if (mesh_index > 0)
			{
				uint num_vss = MeshMLViewerCore.NumVertexStreams(core_, mesh_index - 1);
				for (uint stream_index = 0; stream_index < num_vss; ++ stream_index)
				{
					string stream_name = "";
					uint num_usages = MeshMLViewerCore.NumVertexStreamUsages(core_, mesh_index - 1, stream_index);
					for (uint usage_index = 0; usage_index < num_usages; ++usage_index)
					{
						uint usage = MeshMLViewerCore.VertexStreamUsage(core_, mesh_index - 1, stream_index, usage_index);
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

				uint mat_id = MeshMLViewerCore.MaterialID(core_, mesh_index - 1);
				float[] temp = new float[3];
				Marshal.Copy(MeshMLViewerCore.AmbientMaterial(core_, mat_id), temp, 0, 3);
				properties_obj_.ambient = Color.FromArgb(255, (byte)Math.Max(Math.Min(temp[0] * 255 + 0.5f, 255), 0),
					(byte)Math.Max(Math.Min(temp[1] * 255 + 0.5f, 255), 0),
					(byte)Math.Max(Math.Min(temp[2] * 255 + 0.5f, 255), 0));
				Marshal.Copy(MeshMLViewerCore.DiffuseMaterial(core_, mat_id), temp, 0, 3);
				properties_obj_.diffuse = Color.FromArgb(255, (byte)Math.Max(Math.Min(temp[0] * 255 + 0.5f, 255), 0),
					(byte)Math.Max(Math.Min(temp[1] * 255 + 0.5f, 255), 0),
					(byte)Math.Max(Math.Min(temp[2] * 255 + 0.5f, 255), 0));
				Marshal.Copy(MeshMLViewerCore.SpecularMaterial(core_, mat_id), temp, 0, 3);
				properties_obj_.specular = Color.FromArgb(255, (byte)Math.Max(Math.Min(temp[0] * 255 + 0.5f, 255), 0),
					(byte)Math.Max(Math.Min(temp[1] * 255 + 0.5f, 255), 0),
					(byte)Math.Max(Math.Min(temp[2] * 255 + 0.5f, 255), 0));
				Marshal.Copy(MeshMLViewerCore.EmitMaterial(core_, mat_id), temp, 0, 3);
				properties_obj_.emit = Color.FromArgb(255, (byte)Math.Max(Math.Min(temp[0] * 255 + 0.5f, 255), 0),
					(byte)Math.Max(Math.Min(temp[1] * 255 + 0.5f, 255), 0),
					(byte)Math.Max(Math.Min(temp[2] * 255 + 0.5f, 255), 0));
				properties_obj_.shininess = MeshMLViewerCore.ShininessMaterial(core_, mat_id);
				properties_obj_.opacity = MeshMLViewerCore.OpacityMaterial(core_, mat_id);

				properties_obj_.diffuse_tex = Marshal.PtrToStringAnsi(MeshMLViewerCore.DiffuseTexture(core_, mat_id));
				properties_obj_.specular_tex = Marshal.PtrToStringAnsi(MeshMLViewerCore.SpecularTexture(core_, mat_id));
				properties_obj_.shininess_tex = Marshal.PtrToStringAnsi(MeshMLViewerCore.ShininessTexture(core_, mat_id));
				properties_obj_.bump_tex = Marshal.PtrToStringAnsi(MeshMLViewerCore.BumpTexture(core_, mat_id));
				properties_obj_.height_tex = Marshal.PtrToStringAnsi(MeshMLViewerCore.HeightTexture(core_, mat_id));
				properties_obj_.emit_tex = Marshal.PtrToStringAnsi(MeshMLViewerCore.EmitTexture(core_, mat_id));
				properties_obj_.opacity_tex = Marshal.PtrToStringAnsi(MeshMLViewerCore.OpacityTexture(core_, mat_id));
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
				properties_obj_.bump_tex = "";
				properties_obj_.height_tex = "";
				properties_obj_.emit_tex = "";
				properties_obj_.opacity_tex = "";
			}

			properties.SelectedObject = properties_obj_;
		}

		private void PropertyGridValueChanged(object sender, Xceed.Wpf.Toolkit.PropertyGrid.PropertyValueChangedEventArgs e)
		{
			Xceed.Wpf.Toolkit.PropertyGrid.PropertyItem item = e.OriginalSource as Xceed.Wpf.Toolkit.PropertyGrid.PropertyItem;
			if (0 == item.PropertyOrder)
			{
				uint mesh_index = 0;
				for (; mesh_index < MeshItemsSource.items.Count; ++ mesh_index)
				{
					if (MeshItemsSource.items[(int)mesh_index].DisplayName == (e.NewValue as string))
					{
						break;
					}
				}

				this.UpdateMeshProperties(mesh_index);
			}
		}
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
