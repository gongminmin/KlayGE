using System.Collections.ObjectModel;
using System.ComponentModel;

namespace MtlEditor
{
	/// <summary>
	/// A UI-friendly wrapper around a MaterialEntity object.
	/// </summary>
	public class MaterialEntityViewModel : INotifyPropertyChanged
	{
		public MaterialEntityViewModel(MainWindow wnd, MaterialEntity entity)
			: this(wnd, entity, true)
		{
		}

		public MaterialEntityViewModel(MainWindow wnd, MaterialEntity entity, bool is_expanded)
		{
			wnd_ = wnd;
			entity_ = entity;
			is_expanded_ = is_expanded;

			foreach (var child in entity_.Children)
			{
				children_.Add(new MaterialEntityViewModel(wnd, child, false));
			}
		}

		public ObservableCollection<MaterialEntityViewModel> Children
		{
			get
			{
				return children_;
			}
		}

		public string Name
		{
			get
			{
				return entity_.Name;
			}
		}

		public MaterialEntity Entity
		{
			get
			{
				return entity_;
			}
		}

		/// <summary>
		/// Gets/sets whether the TreeViewItem 
		/// associated with this object is expanded.
		/// </summary>
		public bool IsExpanded
		{
			get
			{
				return is_expanded_;
			}
			set
			{
				if (is_expanded_ != value)
				{
					is_expanded_ = value;
					this.OnPropertyChanged("IsExpanded");
				}
			}
		}

		/// <summary>
		/// Gets/sets whether the TreeViewItem 
		/// associated with this object is selected.
		/// </summary>
		public bool IsSelected
		{
			get
			{
				return is_selected_;
			}
			set
			{
				if (is_selected_ != value)
				{
					this.SelectedInternal(value);

					if (is_selected_)
					{
						wnd_.SelectMaterialEntity(entity_.ID);
					}
				}
			}
		}

		public void SelectedInternal(bool value)
		{
			is_selected_ = value;
			this.OnPropertyChanged("IsSelected");
		}

		public event PropertyChangedEventHandler PropertyChanged;

		protected virtual void OnPropertyChanged(string property_name)
		{
			if (this.PropertyChanged != null)
			{
				this.PropertyChanged(this, new PropertyChangedEventArgs(property_name));
			}
		}

		private MainWindow wnd_;

		private readonly ObservableCollection<MaterialEntityViewModel> children_ = new ObservableCollection<MaterialEntityViewModel>();
		private readonly MaterialEntity entity_;

		private bool is_expanded_;
		private bool is_selected_ = false;
	}
}