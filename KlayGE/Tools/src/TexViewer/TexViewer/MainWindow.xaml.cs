using System;
using System.ComponentModel;
using System.Windows;
using System.Windows.Controls.Ribbon;
using System.Windows.Forms;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using Xceed.Wpf.Toolkit.PropertyGrid.Attributes;

namespace TexViewer
{
	/// <summary>
	/// Interaction logic for MainWindow.xaml
	/// </summary>
	public partial class MainWindow : RibbonWindow
	{
		enum TextureProperties
		{
			TP_TextureType = 0,
			TP_ArraySize,
			TP_NumMipmaps,
			TP_Width,
			TP_Height,
			TP_Depth,
			TP_Format,
			TP_ArrayIndex,
			TP_Face,
			TP_DepthIndex,
			TP_MipmapLevel,
			TP_Stops,
			TP_Zoom
		};

		[CategoryOrder("Properties", 0)]
		[CategoryOrder("Displays", 1)]
		public class TexturePropertyTypes
		{
			[Category("Properties")]
			[DisplayName("Type")]
			[ItemsSource(typeof(TextureTypesSource))]
			[PropertyOrder((int)TextureProperties.TP_TextureType)]
			public string Type { get; set; }

			[Category("Properties")]
			[DisplayName("Array Size")]
			[PropertyOrder((int)TextureProperties.TP_ArraySize)]
			public uint ArraySize { get; set; }

			[Category("Properties")]
			[DisplayName("Mipmaps")]
			[PropertyOrder((int)TextureProperties.TP_NumMipmaps)]
			public uint NumMipmaps { get; set; }

			[Category("Properties")]
			[DisplayName("Width")]
			[PropertyOrder((int)TextureProperties.TP_Width)]
			public uint Width { get; set; }

			[Category("Properties")]
			[DisplayName("Height")]
			[PropertyOrder((int)TextureProperties.TP_Depth)]
			public uint Height { get; set; }

			[Category("Properties")]
			[DisplayName("Depth")]
			[PropertyOrder((int)TextureProperties.TP_Format)]
			public uint Depth { get; set; }

			[Category("Properties")]
			[DisplayName("Format")]
			[ItemsSource(typeof(ElementFormatSource))]
			[PropertyOrder((int)TextureProperties.TP_Format)]
			public string Format { get; set; }

			[Category("Displays")]
			[DisplayName("Array Index")]
			[PropertyOrder((int)TextureProperties.TP_ArrayIndex)]
			[ItemsSource(typeof(TexturArrayIndexSource))]
			public uint ArrayIndex { get; set; }

			[Category("Displays")]
			[DisplayName("Face")]
			[PropertyOrder((int)TextureProperties.TP_Face)]
			[ItemsSource(typeof(TextureFaceSource))]
			public string Face { get; set; }

			[Category("Displays")]
			[DisplayName("Depth Index")]
			[PropertyOrder((int)TextureProperties.TP_DepthIndex)]
			[ItemsSource(typeof(TexturDepthIndexSource))]
			public uint DepthIndex { get; set; }

			[Category("Displays")]
			[DisplayName("Mipmap Level")]
			[PropertyOrder((int)TextureProperties.TP_MipmapLevel)]
			[ItemsSource(typeof(TexturMipmapLevelSource))]
			public uint MipmapLevel { get; set; }

			[Category("Displays")]
			[DisplayName("Stops")]
			[PropertyOrder((int)TextureProperties.TP_Stops)]
			[Editor(typeof(StopsSliderUserControlEditor), typeof(StopsSliderUserControlEditor))]
			public float Stops { get; set; }

			[Category("Displays")]
			[DisplayName("Zoom")]
			[PropertyOrder((int)TextureProperties.TP_Zoom)]
			[ItemsSource(typeof(ZoomSource))]
			public string Zoom { get; set; }
		}

		public MainWindow()
		{
			InitializeComponent();

			DataContext = this;

			properties_obj_ = new TexturePropertyTypes();
			properties_obj_.Stops = 0;
			properties_obj_.Zoom = "1 x";
			properties.SelectedObject = properties_obj_;

			save.IsEnabled = false;
			save_as.IsEnabled = false;
			r_channel.IsEnabled = false;
			g_channel.IsEnabled = false;
			b_channel.IsEnabled = false;
			a_channel.IsEnabled = false;
			properties.IsEnabled = false;

			Uri iconUri = new Uri("pack://application:,,,/Images/klayge_logo.ico", UriKind.RelativeOrAbsolute);
			this.Icon = BitmapFrame.Create(iconUri);
		}

		void MainWindowLoaded(object sender, RoutedEventArgs e)
		{
			IntPtr wnd = editor_wnd.Handle;
			core_ = new KlayGE.TexViewerCoreWrapper(wnd);

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

		private void OpenTexture(string file_name)
		{
			core_.OpenTexture(file_name);
			this.FileNameChanged(file_name);

			save.IsEnabled = true;
			save_as.IsEnabled = true;
			r_channel.IsEnabled = true;
			r_channel.IsChecked = true;
			g_channel.IsEnabled = true;
			g_channel.IsChecked = true;
			b_channel.IsEnabled = true;
			b_channel.IsChecked = true;
			a_channel.IsEnabled = true;
			a_channel.IsChecked = true;
			properties.IsEnabled = true;

			active_array_index_ = 0;
			active_face_ = 0;
			active_depth_index_ = 0;
			active_mipmap_level_ = 0;
			offset_x_ = 0;
			offset_y_ = 0;
			center_pt_x_ = core_.Width(active_mipmap_level_) / 2.0f;
			center_pt_y_ = core_.Height(active_mipmap_level_) / 2.0f;
			stops_ = 0;
			zoom_index_ = 8;
			r_on_ = true;
			g_on_ = true;
			b_on_ = true;
			a_on_ = true;

			TexturArrayIndexSource.items.Clear();
			for (uint i = 0; i < core_.ArraySize(); ++ i)
			{
				TexturArrayIndexSource.items.Add(i.ToString());
			}

			TexturDepthIndexSource.items.Clear();
			for (uint i = 0; i < core_.Depth(active_mipmap_level_); ++ i)
			{
				TexturDepthIndexSource.items.Add(i.ToString());
			}

			TexturMipmapLevelSource.items.Clear();
			for (uint i = 0; i < core_.NumMipmaps(); ++ i)
			{
				TexturMipmapLevelSource.items.Add(i.ToString());
			}

			this.UpdateTextureProperties();
		}

		private void OpenClick(object sender, RoutedEventArgs e)
		{
			Microsoft.Win32.OpenFileDialog dlg = new Microsoft.Win32.OpenFileDialog();

			dlg.DefaultExt = ".dds";
			dlg.Filter = "All Image Files|*.dds;*.bmp;*.exr;*.gif;*.hdr;*.ico;*.iff;*.jng;*.jpg;*.j2k;*.jxr;*.mng;"
				+ "*.pcx;*.pbm;*.pgm;*.ppm;*.pfm;*.png;*.pict;*.psd;*.raw;*.ras;*.sgi;*.tga;*.tiff;*.wbmp;*.webp;*.xbm;*.xpm;"
				+ "|All Files|*.*";
			dlg.CheckPathExists = true;
			dlg.CheckFileExists = true;
			if (true == dlg.ShowDialog())
			{
				this.OpenTexture(dlg.FileName);
			}
		}

		private void SaveClick(object sender, RoutedEventArgs e)
		{
			core_.SaveAsTexture(opened_file_);
		}

		private void SaveAsClick(object sender, RoutedEventArgs e)
		{
			Microsoft.Win32.SaveFileDialog dlg = new Microsoft.Win32.SaveFileDialog();

			dlg.DefaultExt = ".dds";
			dlg.Filter = "DDS Files (*.dds)|*.dds|All Files|*.*";
			dlg.OverwritePrompt = true;
			if (true == dlg.ShowDialog())
			{
				core_.SaveAsTexture(dlg.FileName);
				this.FileNameChanged(dlg.FileName);
			}
		}

		public bool RValue
		{
			get
			{
				return r_on_;
			}
			set
			{
				r_on_ = value;
				this.UpdateColorMask();
			}
		}

		public bool GValue
		{
			get
			{
				return g_on_;
			}
			set
			{
				g_on_ = value;
				this.UpdateColorMask();
			}
		}

		public bool BValue
		{
			get
			{
				return b_on_;
			}
			set
			{
				b_on_ = value;
				this.UpdateColorMask();
			}
		}

		public bool AValue
		{
			get
			{
				return a_on_;
			}
			set
			{
				a_on_ = value;
				this.UpdateColorMask();
			}
		}

		private void EditorMouseDown(object sender, System.Windows.Forms.MouseEventArgs e)
		{
			if (MouseButtons.Left == e.Button)
			{
				mouse_down_in_wnd_ = true;
				last_mouse_pt_x_ = e.X;
				last_mouse_pt_y_ = e.Y;
			}
		}

		private void EditorMouseUp(object sender, System.Windows.Forms.MouseEventArgs e)
		{
			if (MouseButtons.Left == e.Button)
			{
				if (mouse_down_in_wnd_)
				{
					mouse_down_in_wnd_ = false;

					if (mouse_tracking_mode_)
					{
						mouse_tracking_mode_ = false;
					}
				}
			}
		}

		private void EditorMouseMove(object sender, System.Windows.Forms.MouseEventArgs e)
		{
			float zoom = ZoomSource.zooms[zoom_index_];

			if (MouseButtons.Left == e.Button)
			{
				if (mouse_down_in_wnd_)
				{
					mouse_tracking_mode_ = true;

					offset_x_ += (e.X - last_mouse_pt_x_) / zoom;
					offset_y_ += (e.Y - last_mouse_pt_y_) / zoom;
					last_mouse_pt_x_ = e.X;
					last_mouse_pt_y_ = e.Y;
					this.UpdateMatrix();
				}
			}

			string x_str = "-";
			string y_str = "-";
			string r_str = "-";
			string g_str = "-";
			string b_str = "-";
			string a_str = "-";
			if (opened_file_ != "")
			{
				uint width = core_.Width(active_mipmap_level_);
				uint height = core_.Height(active_mipmap_level_);
				int x = (int)this.ToTextureX(e.X);
				int y = (int)this.ToTextureY(e.Y);
				if ((x >= 0) && (x < width) && (y >= 0) && (y < height))
				{
					x_str = x.ToString();
					y_str = y.ToString();

					float[] clr = core_.RetrieveColor((uint)x, (uint)y);
					r_str = String.Format("{0:F3}", clr[0]);
					g_str = String.Format("{0:F3}", clr[1]);
					b_str = String.Format("{0:F3}", clr[2]);
					a_str = String.Format("{0:F3}", clr[3]);
				}
			}
			pixel_info.Text = "Pixel: " + x_str + " " + y_str;
			color_info.Text = "Color: " + r_str + " " + g_str + " " + b_str + " " + a_str;
		}

		private void EditorMouseWheel(object sender, System.Windows.Forms.MouseEventArgs e)
		{
			center_pt_x_ = this.ToTextureX(e.X);
			center_pt_y_ = this.ToTextureY(e.Y);
			old_zoom_index_ = zoom_index_;
			const int WHEEL_DELTA = 120;
			zoom_index_ += e.Delta / WHEEL_DELTA;
			zoom_index_ = Math.Max(Math.Min(zoom_index_, ZoomSource.zooms.Length - 1), 0);

			this.UpdateTextureProperties();
		}

		private void EditorKeyUp(object sender, System.Windows.Input.KeyEventArgs e)
		{
			switch (e.Key)
			{
				case Key.NumPad5:
					offset_x_ = offset_y_ = 0;
					this.UpdateMatrix();
					break;

				case Key.Left:
				case Key.NumPad4:
					{
						float step = 128;
						if ((Keyboard.Modifiers & ModifierKeys.Control) > 0)
						{
							step *= 4;
						}
						offset_x_ -= step;
						this.UpdateMatrix();
					}
					break;

				case Key.Right:
				case Key.NumPad6:
					{
						float step = 128;
						if ((Keyboard.Modifiers & ModifierKeys.Control) > 0)
						{
							step *= 4;
						}
						offset_x_ += step;
						this.UpdateMatrix();
					}
					break;

				case Key.Up:
				case Key.NumPad8:
					{
						float step = 128;
						if ((Keyboard.Modifiers & ModifierKeys.Control) > 0)
						{
							step *= 4;
						}
						offset_y_ -= step;
						this.UpdateMatrix();
					}
					break;

				case Key.Down:
				case Key.NumPad2:
					{
						float step = 128;
						if ((Keyboard.Modifiers & ModifierKeys.Control) > 0)
						{
							step *= 4;
						}
						offset_y_ += step;
						this.UpdateMatrix();
					}
					break;

				case Key.Add:
					if ((Keyboard.Modifiers & ModifierKeys.Control) > 0)
					{
						old_zoom_index_ = zoom_index_;
						++ zoom_index_;
					}
					else if ((Keyboard.Modifiers & ModifierKeys.Shift) > 0)
					{
						stops_ += 0.1f;
					}
					else
					{
						++ stops_;
					}
					this.UpdateTextureProperties();
					break;

				case Key.Subtract:
					if ((Keyboard.Modifiers & ModifierKeys.Control) > 0)
					{
						old_zoom_index_ = zoom_index_;
						-- zoom_index_;
					}
					else if ((Keyboard.Modifiers & ModifierKeys.Shift) > 0)
					{
						stops_ -= 0.1f;
					}
					else
					{
						-- stops_;
					}
					this.UpdateTextureProperties();
					break;
			}
		}

		private void UpdateColorMask()
		{
			core_.ColorMask(r_on_, g_on_, b_on_, a_on_);
		}

		private void UpdateTextureProperties()
		{
			properties_obj_.Type = TextureTypesSource.items[(int)(core_.Type())].DisplayName;
			properties_obj_.ArraySize = core_.ArraySize();
			properties_obj_.NumMipmaps = core_.NumMipmaps();
			properties_obj_.Width = core_.Width(active_mipmap_level_);
			properties_obj_.Height = core_.Height(active_mipmap_level_);
			properties_obj_.Depth = core_.Depth(active_mipmap_level_);
			properties_obj_.Format = ElementFormatSource.items[(int)(core_.Format())].DisplayName;
			properties_obj_.ArrayIndex = active_array_index_;
			if (KlayGE.TexViewerCoreWrapper.TextureType.TT_Cube == core_.Type())
			{
				properties_obj_.Face = TextureFaceSource.items[(int)active_face_].DisplayName;
			}
			else
			{
				properties_obj_.Face = "";
			}
			properties_obj_.DepthIndex = active_depth_index_;
			properties_obj_.MipmapLevel = active_mipmap_level_;
			properties_obj_.Stops = stops_;
			properties_obj_.Zoom = ZoomSource.items[zoom_index_].DisplayName;
			this.UpdateMatrix();

			properties.SelectedObject = null;
			properties.SelectedObject = properties_obj_;
		}

		private void PropertyGridValueChanged(object sender, Xceed.Wpf.Toolkit.PropertyGrid.PropertyValueChangedEventArgs e)
		{
			Xceed.Wpf.Toolkit.PropertyGrid.PropertyItem item = e.OriginalSource as Xceed.Wpf.Toolkit.PropertyGrid.PropertyItem;
			switch ((TextureProperties)item.PropertyOrder)
			{
				case TextureProperties.TP_TextureType:
				case TextureProperties.TP_ArraySize:
				case TextureProperties.TP_NumMipmaps:
				case TextureProperties.TP_Width:
				case TextureProperties.TP_Height:
				case TextureProperties.TP_Depth:
				case TextureProperties.TP_Format:
					break;

				case TextureProperties.TP_ArrayIndex:
					active_array_index_ = properties_obj_.ArrayIndex;
					core_.ArrayIndex(active_array_index_);
					break;

				case TextureProperties.TP_Face:
					if (KlayGE.TexViewerCoreWrapper.TextureType.TT_Cube == core_.Type())
					{
						for (int i = 0; i < ZoomSource.zooms.Length; ++ i)
						{
							if (TextureFaceSource.items[i].DisplayName == properties_obj_.Face)
							{
								active_face_ = (uint)i;
								core_.Face(active_face_);
								break;
							}
						}
					}
					break;

				case TextureProperties.TP_DepthIndex:
					active_depth_index_ = properties_obj_.DepthIndex;
					core_.DepthIndex(active_depth_index_);
					break;

				case TextureProperties.TP_MipmapLevel:
					active_mipmap_level_ = properties_obj_.MipmapLevel;
					core_.MipmapLevel(active_mipmap_level_);
					this.UpdateMatrix();
					if (core_.Depth(active_mipmap_level_) > 1)
					{
						TexturDepthIndexSource.items.Clear();
						for (uint i = 0; i < core_.Depth(active_mipmap_level_); ++ i)
						{
							TexturDepthIndexSource.items.Add(i.ToString());
						}
					}
					active_depth_index_ = Math.Min(active_depth_index_, core_.Depth(active_mipmap_level_));
					break;

				case TextureProperties.TP_Stops:
					stops_ = properties_obj_.Stops;
					core_.Stops(stops_);
					break;

				case TextureProperties.TP_Zoom:
					for (int i = 0; i < ZoomSource.zooms.Length; ++ i)
					{
						if (ZoomSource.items[i].DisplayName == properties_obj_.Zoom)
						{
							old_zoom_index_ = zoom_index_;
							zoom_index_ = i;
							this.UpdateMatrix();
							break;
						}
					}
					break;

				default:
					break;
			}
		}

		private void FileNameChanged(string name)
		{
			opened_file_ = name.Replace('\\', '/');
			int dot_offset = opened_file_.LastIndexOf('.');
			if (".dds" == opened_file_.Substring(dot_offset))
			{
				int slash_offset = opened_file_.LastIndexOf('/') + 1;
				doc1.Title = opened_file_.Substring(slash_offset, dot_offset - slash_offset);
			}
		}

		private void UpdateMatrix()
		{
			uint width = core_.Width(active_mipmap_level_);
			uint height = core_.Height(active_mipmap_level_);
			float old_zoom = ZoomSource.zooms[old_zoom_index_];
			float new_zoom = ZoomSource.zooms[zoom_index_];

			offset_x_ = ((center_pt_x_ - width / 2) + offset_x_) * old_zoom / new_zoom - (center_pt_x_ - width / 2);
			offset_y_ = ((center_pt_y_ - height / 2) + offset_y_) * old_zoom / new_zoom - (center_pt_y_ - height / 2);
			old_zoom_index_ = zoom_index_;

			core_.OffsetAndZoom(offset_x_, offset_y_, new_zoom);
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
				this.OpenTexture(files[0]);
			}
		}

		private float ToTextureX(int win_x)
		{
			float zoom = ZoomSource.zooms[zoom_index_];
			uint width = core_.Width(active_mipmap_level_);
			return (float)((win_x - editor_frame.Width / 2) / zoom - offset_x_ + width / 2);
		}

		private float ToTextureY(int win_y)
		{
			float zoom = ZoomSource.zooms[zoom_index_];
			uint height = core_.Height(active_mipmap_level_);
			return (float)((win_y - editor_frame.Height / 2) / zoom - offset_y_ + height / 2);
		}

		private KlayGE.TexViewerCoreWrapper core_;
		private TexturePropertyTypes properties_obj_;
		private string opened_file_ = "";

		private uint active_array_index_ = 0;
		private uint active_face_ = 0;
		private uint active_depth_index_ = 0;
		private uint active_mipmap_level_ = 0;
		private float offset_x_ = 0;
		private float offset_y_ = 0;
		private float center_pt_x_ = 0;
		private float center_pt_y_ = 0;
		private float stops_ = 0;
		private int zoom_index_ = 8;
		private int old_zoom_index_ = 8;

		private bool r_on_;
		private bool g_on_;
		private bool b_on_;
		private bool a_on_;

		private bool mouse_down_in_wnd_ = false;
		private bool mouse_tracking_mode_ = false;
		private int last_mouse_pt_x_;
		private int last_mouse_pt_y_;
	}

	public class TextureTypesSource : IItemsSource
	{
		static public Xceed.Wpf.Toolkit.PropertyGrid.Attributes.ItemCollection items
			= new Xceed.Wpf.Toolkit.PropertyGrid.Attributes.ItemCollection()
		{
			"1D",
			"2D",
			"3D",
			"Cube"
		};

		public Xceed.Wpf.Toolkit.PropertyGrid.Attributes.ItemCollection GetValues()
		{
			return items;
		}
	}

	public class ElementFormatSource : IItemsSource
	{
		static public Xceed.Wpf.Toolkit.PropertyGrid.Attributes.ItemCollection items
			= new Xceed.Wpf.Toolkit.PropertyGrid.Attributes.ItemCollection()
		{
			"Unknown",

			"A8",

			"R5G6B5",
			"A1RGB5",
			"ARGB4",

			"R8",
			"SIGNED_R8",
			"GR8",
			"SIGNED_GR8",
			"BGR8",
			"SIGNED_BGR8",
			"ARGB8",
			"ABGR8",
			"SIGNED_ABGR8",
			"A2BGR10",
			"SIGNED_A2BGR10",

			"R8UI",
			"R8I",
			"GR8UI",
			"GR8I",
			"BGR8UI",
			"BGR8I",
			"ABGR8UI",
			"ABGR8I",
			"A2BGR10UI",
			"A2BGR10I",

			"R16",
			"SIGNED_R16",
			"GR16",
			"SIGNED_GR16",
			"BGR16",
			"SIGNED_BGR16",
			"ABGR16",
			"SIGNED_ABGR16",
			"R32",
			"SIGNED_R32",
			"GR32",
			"SIGNED_GR32",
			"BGR32",
			"SIGNED_BGR32",
			"ABGR32",
			"SIGNED_ABGR32",

			"R16UI",
			"R16I",
			"GR16UI",
			"GR16I",
			"BGR16UI",
			"BGR16I",
			"ABGR16UI",
			"ABGR16I",
			"R32UI",
			"R32I",
			"GR32UI",
			"GR32I",
			"BGR32UI",
			"BGR32I",
			"ABGR32UI",
			"ABGR32I",

			"R16F",
			"GR16F",
			"B10G11R11F",
			"BGR16F",
			"ABGR16F",
			"R32F",
			"GR32F",
			"BGR32F",
			"ABGR32F",

			"BC1",
			"SIGNED_BC1",
			"BC2",
			"SIGNED_BC2",
			"BC3",
			"SIGNED_BC3",
			"BC4",
			"SIGNED_BC4",
			"BC5",
			"SIGNED_BC5",
			"BC6",
			"SIGNED_BC6",
			"BC7",

			"ETC1",
			"ETC2_R11",
			"SIGNED_ETC2_R11",
			"ETC2_GR11",
			"SIGNED_ETC2_GR11",
			"ETC2_BGR8",
			"ETC2_BGR8_SRGB",
			"ETC2_A1BGR8",
			"ETC2_A1BGR8_SRGB",
			"ETC2_ABGR8",
			"ETC2_ABGR8_SRGB",

			"D16",
			"D24S8",
			"D32F",

			"ARGB8_SRGB",
			"ABGR8_SRGB",
			"BC1_SRGB",
			"BC2_SRGB",
			"BC3_SRGB",
			"BC4_SRGB",
			"BC5_SRGB",
			"BC7_SRGB"
		};

		public Xceed.Wpf.Toolkit.PropertyGrid.Attributes.ItemCollection GetValues()
		{
			return items;
		}
	}

	public class TexturArrayIndexSource : IItemsSource
	{
		static public Xceed.Wpf.Toolkit.PropertyGrid.Attributes.ItemCollection items
			= new Xceed.Wpf.Toolkit.PropertyGrid.Attributes.ItemCollection();

		public Xceed.Wpf.Toolkit.PropertyGrid.Attributes.ItemCollection GetValues()
		{
			return items;
		}
	}

	public class TextureFaceSource : IItemsSource
	{
		static public Xceed.Wpf.Toolkit.PropertyGrid.Attributes.ItemCollection items
			= new Xceed.Wpf.Toolkit.PropertyGrid.Attributes.ItemCollection()
		{
			"Pos X",
			"Neg X",
			"Pos Y",
			"Neg Y",
			"Pos Z",
			"Neg Z"
		};

		public Xceed.Wpf.Toolkit.PropertyGrid.Attributes.ItemCollection GetValues()
		{
			return items;
		}
	}

	public class TexturDepthIndexSource : IItemsSource
	{
		static public Xceed.Wpf.Toolkit.PropertyGrid.Attributes.ItemCollection items
			= new Xceed.Wpf.Toolkit.PropertyGrid.Attributes.ItemCollection();

		public Xceed.Wpf.Toolkit.PropertyGrid.Attributes.ItemCollection GetValues()
		{
			return items;
		}
	}

	public class TexturMipmapLevelSource : IItemsSource
	{
		static public Xceed.Wpf.Toolkit.PropertyGrid.Attributes.ItemCollection items
			= new Xceed.Wpf.Toolkit.PropertyGrid.Attributes.ItemCollection();

		public Xceed.Wpf.Toolkit.PropertyGrid.Attributes.ItemCollection GetValues()
		{
			return items;
		}
	}

	public class ZoomSource : IItemsSource
	{
		static public Xceed.Wpf.Toolkit.PropertyGrid.Attributes.ItemCollection items
			= new Xceed.Wpf.Toolkit.PropertyGrid.Attributes.ItemCollection()
		{
			"1/256 x",
			"1/128 x",
			"1/64 x",
			"1/32 x",
			"1/16 x",
			"1/8 x",
			"1/4 x",
			"1/2 x",
			"1 x",
			"2 x",
			"4 x",
			"8 x",
			"16 x",
			"32 x",
			"64 x",
			"128 x",
			"256 x",
			"512 x",
			"1024 x"
		};

		static public float[] zooms = new float[]
		{
			1.0f / 256,
			1.0f / 128,
			1.0f / 64,
			1.0f / 32,
			1.0f / 16,
			1.0f / 8,
			1.0f / 4,
			1.0f / 2,
			1,
			2,
			4,
			8,
			16,
			32,
			64,
			128,
			256,
			512,
			1024
		};

		public Xceed.Wpf.Toolkit.PropertyGrid.Attributes.ItemCollection GetValues()
		{
			return items;
		}
	}
}
