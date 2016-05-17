/**
 * @file OpenTexUserControlEditor.xaml.cs
 * @author Minmin Gong
 *
 * @section DESCRIPTION
 *
 * This source file is part of KlayGE
 * For the latest info, see http://www.klayge.org
 *
 * @section LICENSE
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 * You may alternatively use this source under the terms of
 * the KlayGE Proprietary License (KPL). You can obtained such a license
 * from http://www.klayge.org/licensing/.
 */

using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using Xceed.Wpf.Toolkit.PropertyGrid.Editors;

namespace KGEditor
{
	/// <summary>
	/// Interaction logic for OpenTexUserControlEditor.xaml
	/// </summary>
	public partial class OpenTexUserControlEditor : UserControl, ITypeEditor
	{
		public OpenTexUserControlEditor()
		{
			InitializeComponent();
		}

		public static readonly DependencyProperty ValueProperty = DependencyProperty.Register("Value",
			typeof(string), typeof(OpenTexUserControlEditor),
			new FrameworkPropertyMetadata(null, FrameworkPropertyMetadataOptions.BindsTwoWayByDefault));
		public string Value
		{
			get
			{
				return (string)GetValue(ValueProperty);
			}
			set
			{
				SetValue(ValueProperty, value);
			}
		}

		private void OpenTexClick(object sender, RoutedEventArgs e)
		{
			Microsoft.Win32.OpenFileDialog dlg = new Microsoft.Win32.OpenFileDialog();

			dlg.DefaultExt = ".dds";
			dlg.Filter = "All Texture Files (*.dds)|*.dds|All Files|*.*";
			dlg.CheckPathExists = true;
			dlg.CheckFileExists = true;
			if (true == dlg.ShowDialog())
			{
				Value = dlg.FileName;
			}
		}

		public FrameworkElement ResolveEditor(Xceed.Wpf.Toolkit.PropertyGrid.PropertyItem property_item)
		{
			Binding binding = new Binding("Value");
			binding.Source = property_item;
			binding.Mode = property_item.IsReadOnly ? BindingMode.OneWay : BindingMode.TwoWay;
			BindingOperations.SetBinding(this, OpenTexUserControlEditor.ValueProperty, binding);
			return this;
		}
	}
}
