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
		public static extern uint NumVertexStreams(IntPtr core, uint mesh_id);
		[DllImport(CORE_NAME)]
		public static extern uint NumVertexStreamUsages(IntPtr core, uint mesh_id, uint stream_index);
		[DllImport(CORE_NAME)]
		public static extern uint VertexStreamUsage(IntPtr core, uint mesh_id, uint stream_index,
			uint usage_index);
		[DllImport(CORE_NAME)]
		public static extern uint MaterialID(IntPtr core, uint mesh_id);
		[DllImport(CORE_NAME)]
		public static extern IntPtr AmbientMaterial(IntPtr core, uint mtl_id);
		[DllImport(CORE_NAME)]
		public static extern IntPtr DiffuseMaterial(IntPtr core, uint mtl_id);
		[DllImport(CORE_NAME)]
		public static extern IntPtr SpecularMaterial(IntPtr core, uint mtl_id);
		[DllImport(CORE_NAME)]
		public static extern float ShininessMaterial(IntPtr core, uint mtl_id);
		[DllImport(CORE_NAME)]
		public static extern IntPtr EmitMaterial(IntPtr core, uint mtl_id);
		[DllImport(CORE_NAME)]
		public static extern float OpacityMaterial(IntPtr core, uint mtl_id);
		[DllImport(CORE_NAME)]
		public static extern IntPtr DiffuseTexture(IntPtr core, uint mtl_id);
		[DllImport(CORE_NAME)]
		public static extern IntPtr SpecularTexture(IntPtr core, uint mtl_id);
		[DllImport(CORE_NAME)]
		public static extern IntPtr ShininessTexture(IntPtr core, uint mtl_id);
		[DllImport(CORE_NAME)]
		public static extern IntPtr NormalTexture(IntPtr core, uint mtl_id);
		[DllImport(CORE_NAME)]
		public static extern IntPtr HeightTexture(IntPtr core, uint mtl_id);
		[DllImport(CORE_NAME)]
		public static extern IntPtr EmitTexture(IntPtr core, uint mtl_id);
		[DllImport(CORE_NAME)]
		public static extern IntPtr OpacityTexture(IntPtr core, uint mtl_id);
		[DllImport(CORE_NAME)]
		public static extern uint SelectedMesh(IntPtr core);
		[DllImport(CORE_NAME)]
		public static extern void SelectMesh(IntPtr core, uint mesh_id);
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
		public uint NumVertexStreams(uint mesh_id)
		{
			return MeshMLViewerCoreImporter.NumVertexStreams(core_, mesh_id);
		}
		public uint NumVertexStreamUsages(uint mesh_id, uint stream_index)
		{
			return MeshMLViewerCoreImporter.NumVertexStreamUsages(core_, mesh_id, stream_index);
		}
		public uint VertexStreamUsage(uint mesh_id, uint stream_index, uint usage_index)
		{
			return MeshMLViewerCoreImporter.VertexStreamUsage(core_, mesh_id, stream_index, usage_index);
		}
		public uint MaterialID(uint mesh_id)
		{
			return MeshMLViewerCoreImporter.MaterialID(core_, mesh_id);
		}
		public Color AmbientMaterial(uint mtl_id)
		{
			return this.IntPtrToColor(MeshMLViewerCoreImporter.AmbientMaterial(core_, mtl_id));
		}
		public Color DiffuseMaterial(uint mtl_id)
		{
			return this.IntPtrToColor(MeshMLViewerCoreImporter.DiffuseMaterial(core_, mtl_id));
		}
		public Color SpecularMaterial(uint mtl_id)
		{
			return this.IntPtrToColor(MeshMLViewerCoreImporter.SpecularMaterial(core_, mtl_id));
		}
		public float ShininessMaterial(uint mtl_id)
		{
			return MeshMLViewerCoreImporter.ShininessMaterial(core_, mtl_id);
		}
		public Color EmitMaterial(uint mtl_id)
		{
			return this.IntPtrToColor(MeshMLViewerCoreImporter.EmitMaterial(core_, mtl_id));
		}
		public float OpacityMaterial(uint mtl_id)
		{
			return MeshMLViewerCoreImporter.OpacityMaterial(core_, mtl_id);
		}
		public string DiffuseTexture(uint mtl_id)
		{
			return Marshal.PtrToStringAnsi(MeshMLViewerCoreImporter.DiffuseTexture(core_, mtl_id));
		}
		public string SpecularTexture(uint mtl_id)
		{
			return Marshal.PtrToStringAnsi(MeshMLViewerCoreImporter.SpecularTexture(core_, mtl_id));
		}
		public string ShininessTexture(uint mtl_id)
		{
			return Marshal.PtrToStringAnsi(MeshMLViewerCoreImporter.ShininessTexture(core_, mtl_id));
		}
		public string NormalTexture(uint mtl_id)
		{
			return Marshal.PtrToStringAnsi(MeshMLViewerCoreImporter.NormalTexture(core_, mtl_id));
		}
		public string HeightTexture(uint mtl_id)
		{
			return Marshal.PtrToStringAnsi(MeshMLViewerCoreImporter.HeightTexture(core_, mtl_id));
		}
		public string EmitTexture(uint mtl_id)
		{
			return Marshal.PtrToStringAnsi(MeshMLViewerCoreImporter.EmitTexture(core_, mtl_id));
		}
		public string OpacityTexture(uint mtl_id)
		{
			return Marshal.PtrToStringAnsi(MeshMLViewerCoreImporter.OpacityTexture(core_, mtl_id));
		}
		public uint SelectedMesh()
		{
			return MeshMLViewerCoreImporter.SelectedMesh(core_);
		}
		public void SelectMesh(uint mesh_id)
		{
			MeshMLViewerCoreImporter.SelectMesh(core_, mesh_id);
		}

		private Color IntPtrToColor(IntPtr clr)
		{
			float[] temp = new float[3];
			Marshal.Copy(clr, temp, 0, 3);
			for (int i = 0; i < 3; ++ i)
			{
				temp[i] = this.LinearToSRGB(temp[i]);
			}
			return Color.FromArgb(255, (byte)Math.Max(Math.Min(temp[0] * 255 + 0.5f, 255), 0),
					(byte)Math.Max(Math.Min(temp[1] * 255 + 0.5f, 255), 0),
					(byte)Math.Max(Math.Min(temp[2] * 255 + 0.5f, 255), 0));
		}

		private float LinearToSRGB(float linear)
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

		private IntPtr core_;
	}
}
