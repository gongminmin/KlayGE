/**
 * @file SliderUserControlEditor.xaml.cs
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

using System;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using Xceed.Wpf.Toolkit.PropertyGrid.Editors;

namespace MtlEditor
{
	/// <summary>
	/// Interaction logic for SliderUserControlEditor.xaml
	/// </summary>
	public partial class SliderUserControlEditor : UserControl, ITypeEditor
	{
		public static readonly DependencyProperty ValueProperty = DependencyProperty.Register("Value",
			typeof(string), typeof(SliderUserControlEditor),
			new FrameworkPropertyMetadata(null, FrameworkPropertyMetadataOptions.BindsTwoWayByDefault));

		public SliderUserControlEditor()
			: this(0, 1)
		{
		}

		public SliderUserControlEditor(double min, double max)
		{
			InitializeComponent();

			sl.Minimum = min;
			sl.Maximum = max;
		}
		public string Value
		{
			get { return (string)GetValue(ValueProperty); }
			set { SetValue(ValueProperty, value); }
		}

		private void SliderValueChanged(object sender, RoutedPropertyChangedEventArgs<double> e)
		{
			Value = e.NewValue.ToString();
			tb.Text = Value;
		}

		private void TextBoxTextChanged(object sender, TextChangedEventArgs e)
		{
			if (tb.Text != "")
			{
				Value = tb.Text;
				sl.Value = Double.Parse(Value);
			}
		}

		public FrameworkElement ResolveEditor(Xceed.Wpf.Toolkit.PropertyGrid.PropertyItem property_item)
		{
			Binding binding = new Binding("Value");
			binding.Source = property_item;
			binding.Mode = property_item.IsReadOnly ? BindingMode.OneWay : BindingMode.TwoWay;
			BindingOperations.SetBinding(this, SliderUserControlEditor.ValueProperty, binding);
			return this;
		}
	}

	public partial class MultiplierSliderUserControlEditor : SliderUserControlEditor
	{
		public MultiplierSliderUserControlEditor()
			: base(1, 64)
		{
			InitializeComponent();
		}
	}
}
