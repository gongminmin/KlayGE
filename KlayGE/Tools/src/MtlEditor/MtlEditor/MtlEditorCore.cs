using System;
using System.Runtime.InteropServices;
using System.Windows.Media;

namespace MtlEditor
{
	public sealed class MtlEditorCoreImporter
	{
#if DEBUG
		const string CORE_NAME = "MtlEditorCore_d.dll";
#else
		const string CORE_NAME = "MtlEditorCore.dll";
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
		public static extern uint MaterialID(IntPtr core, uint mesh_index);
		[DllImport(CORE_NAME)]
		public static extern IntPtr GetAmbientMaterial(IntPtr core, uint material_index);
		[DllImport(CORE_NAME)]
		public static extern IntPtr GetDiffuseMaterial(IntPtr core, uint material_index);
		[DllImport(CORE_NAME)]
		public static extern IntPtr GetSpecularMaterial(IntPtr core, uint material_index);
		[DllImport(CORE_NAME)]
		public static extern float GetShininessMaterial(IntPtr core, uint material_index);
		[DllImport(CORE_NAME)]
		public static extern IntPtr GetEmitMaterial(IntPtr core, uint material_index);
		[DllImport(CORE_NAME)]
		public static extern float GetOpacityMaterial(IntPtr core, uint material_index);
		[DllImport(CORE_NAME)]
		public static extern IntPtr GetDiffuseTexture(IntPtr core, uint material_index);
		[DllImport(CORE_NAME)]
		public static extern IntPtr GetSpecularTexture(IntPtr core, uint material_index);
		[DllImport(CORE_NAME)]
		public static extern IntPtr GetShininessTexture(IntPtr core, uint material_index);
		[DllImport(CORE_NAME)]
		public static extern IntPtr GetBumpTexture(IntPtr core, uint material_index);
		[DllImport(CORE_NAME)]
		public static extern IntPtr GetHeightTexture(IntPtr core, uint material_index);
		[DllImport(CORE_NAME)]
		public static extern IntPtr GetEmitTexture(IntPtr core, uint material_index);
		[DllImport(CORE_NAME)]
		public static extern IntPtr GetOpacityTexture(IntPtr core, uint material_index);
		[DllImport(CORE_NAME)]
		public static extern uint SelectedMesh(IntPtr core);
		[DllImport(CORE_NAME)]
		public static extern void SelectMesh(IntPtr core, uint mesh_index);
	}

	public sealed class MtlEditorCore
	{
		public MtlEditorCore(IntPtr native_wnd)
		{
			core_ = MtlEditorCoreImporter.Create(native_wnd);
		}
		~MtlEditorCore()
		{
			if (core_ != IntPtr.Zero)
			{
				this.Destroy();
			}
		}
		
		public void Destroy()
		{
			MtlEditorCoreImporter.Destroy(core_);
			core_ = IntPtr.Zero;
		}

		public void Refresh()
		{
			MtlEditorCoreImporter.Refresh(core_);
		}
		public void Resize(uint width, uint height)
		{
			MtlEditorCoreImporter.Resize(core_, width, height);
		}
		public void OpenModel(string name)
		{
			MtlEditorCoreImporter.OpenModel(core_, name);
		}
		public void SaveModel(string name)
		{
			MtlEditorCoreImporter.SaveModel(core_, name);
		}
		public uint NumFrames()
		{
			return MtlEditorCoreImporter.NumFrames(core_);
		}
		public void CurrFrame(float frame)
		{
			MtlEditorCoreImporter.CurrFrame(core_, frame);
		}
		public float ModelFrameRate()
		{
			return MtlEditorCoreImporter.ModelFrameRate(core_);
		}
		public void SkinningOn(int on)
		{
			MtlEditorCoreImporter.SkinningOn(core_, on);
		}
		public void FPSCameraOn(int on)
		{
			MtlEditorCoreImporter.FPSCameraOn(core_, on);
		}
		public void Visualize(int index)
		{
			MtlEditorCoreImporter.Visualize(core_, index);
		}
		public void MouseMove(int x, int y, uint button)
		{
			MtlEditorCoreImporter.MouseMove(core_, x, y, button);
		}
		public void MouseUp(int x, int y, uint button)
		{
			MtlEditorCoreImporter.MouseUp(core_, x, y, button);
		}
		public void MouseDown(int x, int y, uint button)
		{
			MtlEditorCoreImporter.MouseDown(core_, x, y, button);
		}
		public void KeyPress(int key)
		{
			MtlEditorCoreImporter.KeyPress(core_, key);
		}
		public uint NumMeshes()
		{
			return MtlEditorCoreImporter.NumMeshes(core_);
		}
		public string MeshName(uint index)
		{
			return Marshal.PtrToStringUni(MtlEditorCoreImporter.MeshName(core_, index));
		}
		public uint MaterialID(uint mesh_index)
		{
			return MtlEditorCoreImporter.MaterialID(core_, mesh_index);
		}
		public Color AmbientMaterial(uint material_index)
		{
			return IntPtrToColor(MtlEditorCoreImporter.GetAmbientMaterial(core_, material_index));
		}
		public Color DiffuseMaterial(uint material_index)
		{
			return IntPtrToColor(MtlEditorCoreImporter.GetDiffuseMaterial(core_, material_index));
		}
		public Color SpecularMaterial(uint material_index)
		{
			return IntPtrToColor(MtlEditorCoreImporter.GetSpecularMaterial(core_, material_index));
		}
		public float ShininessMaterial(uint material_index)
		{
			return MtlEditorCoreImporter.GetShininessMaterial(core_, material_index);
		}
		public Color EmitMaterial(uint material_index)
		{
			return IntPtrToColor(MtlEditorCoreImporter.GetEmitMaterial(core_, material_index));
		}
		public float OpacityMaterial(uint material_index)
		{
			return MtlEditorCoreImporter.GetOpacityMaterial(core_, material_index);
		}
		public string DiffuseTexture(uint material_index)
		{
			return Marshal.PtrToStringAnsi(MtlEditorCoreImporter.GetDiffuseTexture(core_, material_index));
		}
		public string SpecularTexture(uint material_index)
		{
			return Marshal.PtrToStringAnsi(MtlEditorCoreImporter.GetSpecularTexture(core_, material_index));
		}
		public string ShininessTexture(uint material_index)
		{
			return Marshal.PtrToStringAnsi(MtlEditorCoreImporter.GetShininessTexture(core_, material_index));
		}
		public string BumpTexture(uint material_index)
		{
			return Marshal.PtrToStringAnsi(MtlEditorCoreImporter.GetBumpTexture(core_, material_index));
		}
		public string HeightTexture(uint material_index)
		{
			return Marshal.PtrToStringAnsi(MtlEditorCoreImporter.GetHeightTexture(core_, material_index));
		}
		public string EmitTexture(uint material_index)
		{
			return Marshal.PtrToStringAnsi(MtlEditorCoreImporter.GetEmitTexture(core_, material_index));
		}
		public string OpacityTexture(uint material_index)
		{
			return Marshal.PtrToStringAnsi(MtlEditorCoreImporter.GetOpacityTexture(core_, material_index));
		}
		public uint SelectedMesh()
		{
			return MtlEditorCoreImporter.SelectedMesh(core_);
		}
		public void SelectMesh(uint mesh_index)
		{
			MtlEditorCoreImporter.SelectMesh(core_, mesh_index);
		}

		private Color IntPtrToColor(IntPtr clr)
		{
			float[] temp = new float[3];
			Marshal.Copy(clr, temp, 0, 3);
			return Color.FromArgb(255, (byte)Math.Max(Math.Min(temp[0] * 255 + 0.5f, 255), 0),
					(byte)Math.Max(Math.Min(temp[1] * 255 + 0.5f, 255), 0),
					(byte)Math.Max(Math.Min(temp[2] * 255 + 0.5f, 255), 0));
		}

		private IntPtr core_;
	}
}
