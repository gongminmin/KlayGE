using System;
using System.Runtime.InteropServices;
using System.Windows.Media;

namespace MeshMLViewer
{
	public sealed class MeshMLViewerCoreImporter
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

	public sealed class MeshMLViewerCore
	{
		public MeshMLViewerCore(IntPtr native_wnd)
		{
			core_ = MeshMLViewerCoreImporter.Create(native_wnd);
		}
		~MeshMLViewerCore()
		{
			if (core_ != IntPtr.Zero)
			{
				this.Destroy();
			}
		}
		
		public void Destroy()
		{
			MeshMLViewerCoreImporter.Destroy(core_);
			core_ = IntPtr.Zero;
		}

		public void Refresh()
		{
			MeshMLViewerCoreImporter.Refresh(core_);
		}
		public void Resize(uint width, uint height)
		{
			MeshMLViewerCoreImporter.Resize(core_, width, height);
		}
		public void OpenModel(string name)
		{
			MeshMLViewerCoreImporter.OpenModel(core_, name);
		}
		public void SaveModel(string name)
		{
			MeshMLViewerCoreImporter.SaveModel(core_, name);
		}
		public uint NumFrames()
		{
			return MeshMLViewerCoreImporter.NumFrames(core_);
		}
		public void CurrFrame(float frame)
		{
			MeshMLViewerCoreImporter.CurrFrame(core_, frame);
		}
		public float ModelFrameRate()
		{
			return MeshMLViewerCoreImporter.ModelFrameRate(core_);
		}
		public void SkinningOn(int on)
		{
			MeshMLViewerCoreImporter.SkinningOn(core_, on);
		}
		public void SmoothMeshOn(int on)
		{
			MeshMLViewerCoreImporter.SmoothMeshOn(core_, on);
		}
		public void FPSCameraOn(int on)
		{
			MeshMLViewerCoreImporter.FPSCameraOn(core_, on);
		}
		public void LineModeOn(int on)
		{
			MeshMLViewerCoreImporter.LineModeOn(core_, on);
		}
		public void Visualize(int index)
		{
			MeshMLViewerCoreImporter.Visualize(core_, index);
		}
		public void MouseMove(int x, int y, uint button)
		{
			MeshMLViewerCoreImporter.MouseMove(core_, x, y, button);
		}
		public void MouseUp(int x, int y, uint button)
		{
			MeshMLViewerCoreImporter.MouseUp(core_, x, y, button);
		}
		public void MouseDown(int x, int y, uint button)
		{
			MeshMLViewerCoreImporter.MouseDown(core_, x, y, button);
		}
		public void KeyPress(int key)
		{
			MeshMLViewerCoreImporter.KeyPress(core_, key);
		}
		public uint NumMeshes()
		{
			return MeshMLViewerCoreImporter.NumMeshes(core_);
		}
		public string MeshName(uint index)
		{
			return Marshal.PtrToStringUni(MeshMLViewerCoreImporter.MeshName(core_, index));
		}
		public uint NumVertexStreams(uint mesh_index)
		{
			return MeshMLViewerCoreImporter.NumVertexStreams(core_, mesh_index);
		}
		public uint NumVertexStreamUsages(uint mesh_index, uint stream_index)
		{
			return MeshMLViewerCoreImporter.NumVertexStreamUsages(core_, mesh_index, stream_index);
		}
		public uint VertexStreamUsage(uint mesh_index, uint stream_index, uint usage_index)
		{
			return MeshMLViewerCoreImporter.VertexStreamUsage(core_, mesh_index, stream_index, usage_index);
		}
		public uint MaterialID(uint mesh_index)
		{
			return MeshMLViewerCoreImporter.MaterialID(core_, mesh_index);
		}
		public Color AmbientMaterial(uint material_index)
		{
			return IntPtrToColor(MeshMLViewerCoreImporter.AmbientMaterial(core_, material_index));
		}
		public Color DiffuseMaterial(uint material_index)
		{
			return IntPtrToColor(MeshMLViewerCoreImporter.DiffuseMaterial(core_, material_index));
		}
		public Color SpecularMaterial(uint material_index)
		{
			return IntPtrToColor(MeshMLViewerCoreImporter.SpecularMaterial(core_, material_index));
		}
		public float ShininessMaterial(uint material_index)
		{
			return MeshMLViewerCoreImporter.ShininessMaterial(core_, material_index);
		}
		public Color EmitMaterial(uint material_index)
		{
			return IntPtrToColor(MeshMLViewerCoreImporter.EmitMaterial(core_, material_index));
		}
		public float OpacityMaterial(uint material_index)
		{
			return MeshMLViewerCoreImporter.OpacityMaterial(core_, material_index);
		}
		public string DiffuseTexture(uint material_index)
		{
			return Marshal.PtrToStringAnsi(MeshMLViewerCoreImporter.DiffuseTexture(core_, material_index));
		}
		public string SpecularTexture(uint material_index)
		{
			return Marshal.PtrToStringAnsi(MeshMLViewerCoreImporter.SpecularTexture(core_, material_index));
		}
		public string ShininessTexture(uint material_index)
		{
			return Marshal.PtrToStringAnsi(MeshMLViewerCoreImporter.ShininessTexture(core_, material_index));
		}
		public string BumpTexture(uint material_index)
		{
			return Marshal.PtrToStringAnsi(MeshMLViewerCoreImporter.BumpTexture(core_, material_index));
		}
		public string HeightTexture(uint material_index)
		{
			return Marshal.PtrToStringAnsi(MeshMLViewerCoreImporter.HeightTexture(core_, material_index));
		}
		public string EmitTexture(uint material_index)
		{
			return Marshal.PtrToStringAnsi(MeshMLViewerCoreImporter.EmitTexture(core_, material_index));
		}
		public string OpacityTexture(uint material_index)
		{
			return Marshal.PtrToStringAnsi(MeshMLViewerCoreImporter.OpacityTexture(core_, material_index));
		}
		public uint SelectedMesh()
		{
			return MeshMLViewerCoreImporter.SelectedMesh(core_);
		}
		public void SelectMesh(uint mesh_index)
		{
			MeshMLViewerCoreImporter.SelectMesh(core_, mesh_index);
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
