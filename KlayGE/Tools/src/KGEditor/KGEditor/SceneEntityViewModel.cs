using System.Collections.ObjectModel;
using System.ComponentModel;

namespace KGEditor
{
	/// <summary>
	/// A UI-friendly wrapper around a SceneEntity object.
	/// </summary>
	public class SceneEntityViewModel : INotifyPropertyChanged
	{
		public SceneEntityViewModel(MainWindow wnd, SceneEntity entity)
			: this(wnd, entity, true)
		{
		}

		public SceneEntityViewModel(MainWindow wnd, SceneEntity entity, bool is_expanded)
		{
			wnd_ = wnd;
			entity_ = entity;
			is_expanded_ = is_expanded;

			foreach (var child in entity_.Children)
			{
				children_.Add(new SceneEntityViewModel(wnd, child, false));
			}
		}

		public ObservableCollection<SceneEntityViewModel> Children
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

		public SceneEntity Entity
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
						wnd_.SelectEntity(entity_.Id);
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

		private readonly ObservableCollection<SceneEntityViewModel> children_ = new ObservableCollection<SceneEntityViewModel>();
		private readonly SceneEntity entity_;

		private bool is_expanded_;
		private bool is_selected_ = false;
	}
}