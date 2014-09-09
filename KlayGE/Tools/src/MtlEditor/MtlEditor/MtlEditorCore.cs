/**
 * @file MtlEditorCore.cs
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
		public static extern uint MaterialID(IntPtr core, uint mesh_id);
		[DllImport(CORE_NAME)]
		public static extern IntPtr GetAmbientMaterial(IntPtr core, uint mtl_id);
		[DllImport(CORE_NAME)]
		public static extern IntPtr GetDiffuseMaterial(IntPtr core, uint mtl_id);
		[DllImport(CORE_NAME)]
		public static extern IntPtr GetSpecularMaterial(IntPtr core, uint mtl_id);
		[DllImport(CORE_NAME)]
		public static extern float GetShininessMaterial(IntPtr core, uint mtl_id);
		[DllImport(CORE_NAME)]
		public static extern IntPtr GetEmitMaterial(IntPtr core, uint mtl_id);
		[DllImport(CORE_NAME)]
		public static extern float GetOpacityMaterial(IntPtr core, uint mtl_id);
		[DllImport(CORE_NAME)]
		public static extern IntPtr GetDiffuseTexture(IntPtr core, uint mtl_id);
		[DllImport(CORE_NAME)]
		public static extern IntPtr GetSpecularTexture(IntPtr core, uint mtl_id);
		[DllImport(CORE_NAME)]
		public static extern IntPtr GetShininessTexture(IntPtr core, uint mtl_id);
		[DllImport(CORE_NAME)]
		public static extern IntPtr GetNormalTexture(IntPtr core, uint mtl_id);
		[DllImport(CORE_NAME)]
		public static extern IntPtr GetHeightTexture(IntPtr core, uint mtl_id);
		[DllImport(CORE_NAME)]
		public static extern IntPtr GetEmitTexture(IntPtr core, uint mtl_id);
		[DllImport(CORE_NAME)]
		public static extern IntPtr GetOpacityTexture(IntPtr core, uint mtl_id);
		[DllImport(CORE_NAME)]
		public static extern uint NumHistroyCmds(IntPtr core);
		[DllImport(CORE_NAME)]
		public static extern IntPtr HistroyCmdName(IntPtr core, uint index);
		[DllImport(CORE_NAME)]
		public static extern uint EndCmdIndex(IntPtr core);
		[DllImport(CORE_NAME)]
		public static extern void SetAmbientMaterial(IntPtr core, uint mtl_id, IntPtr value);
		[DllImport(CORE_NAME)]
		public static extern void SetDiffuseMaterial(IntPtr core, uint mtl_id, IntPtr value);
		[DllImport(CORE_NAME)]
		public static extern void SetSpecularMaterial(IntPtr core, uint mtl_id, IntPtr value);
		[DllImport(CORE_NAME)]
		public static extern void SetShininessMaterial(IntPtr core, uint mtl_id, float value);
		[DllImport(CORE_NAME)]
		public static extern void SetEmitMaterial(IntPtr core, uint mtl_id, IntPtr value);
		[DllImport(CORE_NAME)]
		public static extern void SetOpacityMaterial(IntPtr core, uint mtl_id, float value);
		[DllImport(CORE_NAME)]
		public static extern void SetDiffuseTexture(IntPtr core, uint mtl_id, IntPtr name);
		[DllImport(CORE_NAME)]
		public static extern void SetSpecularTexture(IntPtr core, uint mtl_id, IntPtr name);
		[DllImport(CORE_NAME)]
		public static extern void SetShininessTexture(IntPtr core, uint mtl_id, IntPtr name);
		[DllImport(CORE_NAME)]
		public static extern void SetNormalTexture(IntPtr core, uint mtl_id, IntPtr name);
		[DllImport(CORE_NAME)]
		public static extern void SetHeightTexture(IntPtr core, uint mtl_id, IntPtr name);
		[DllImport(CORE_NAME)]
		public static extern void SetEmitTexture(IntPtr core, uint mtl_id, IntPtr name);
		[DllImport(CORE_NAME)]
		public static extern void SetOpacityTexture(IntPtr core, uint mtl_id, IntPtr name);
		[DllImport(CORE_NAME)]
		public static extern uint SelectedMesh(IntPtr core);
		[DllImport(CORE_NAME)]
		public static extern void SelectMesh(IntPtr core, uint mesh_id);
		[DllImport(CORE_NAME)]
		public static extern void Undo(IntPtr core);
		[DllImport(CORE_NAME)]
		public static extern void Redo(IntPtr core);
		[DllImport(CORE_NAME)]
		public static extern void ClearHistroy(IntPtr core);
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
		public uint MaterialID(uint mesh_id)
		{
			return MtlEditorCoreImporter.MaterialID(core_, mesh_id);
		}
		public Color AmbientMaterial(uint mtl_id)
		{
			return this.IntPtrToColor(MtlEditorCoreImporter.GetAmbientMaterial(core_, mtl_id));
		}
		public Color DiffuseMaterial(uint mtl_id)
		{
			return this.IntPtrToColor(MtlEditorCoreImporter.GetDiffuseMaterial(core_, mtl_id));
		}
		public Color SpecularMaterial(uint mtl_id)
		{
			return this.IntPtrToColor(MtlEditorCoreImporter.GetSpecularMaterial(core_, mtl_id));
		}
		public float ShininessMaterial(uint mtl_id)
		{
			return MtlEditorCoreImporter.GetShininessMaterial(core_, mtl_id);
		}
		public Color EmitMaterial(uint mtl_id)
		{
			return this.IntPtrToColor(MtlEditorCoreImporter.GetEmitMaterial(core_, mtl_id));
		}
		public float OpacityMaterial(uint mtl_id)
		{
			return MtlEditorCoreImporter.GetOpacityMaterial(core_, mtl_id);
		}
		public string DiffuseTexture(uint mtl_id)
		{
			return Marshal.PtrToStringAnsi(MtlEditorCoreImporter.GetDiffuseTexture(core_, mtl_id));
		}
		public string SpecularTexture(uint mtl_id)
		{
			return Marshal.PtrToStringAnsi(MtlEditorCoreImporter.GetSpecularTexture(core_, mtl_id));
		}
		public string ShininessTexture(uint mtl_id)
		{
			return Marshal.PtrToStringAnsi(MtlEditorCoreImporter.GetShininessTexture(core_, mtl_id));
		}
		public string NormalTexture(uint mtl_id)
		{
			return Marshal.PtrToStringAnsi(MtlEditorCoreImporter.GetNormalTexture(core_, mtl_id));
		}
		public string HeightTexture(uint mtl_id)
		{
			return Marshal.PtrToStringAnsi(MtlEditorCoreImporter.GetHeightTexture(core_, mtl_id));
		}
		public string EmitTexture(uint mtl_id)
		{
			return Marshal.PtrToStringAnsi(MtlEditorCoreImporter.GetEmitTexture(core_, mtl_id));
		}
		public string OpacityTexture(uint mtl_id)
		{
			return Marshal.PtrToStringAnsi(MtlEditorCoreImporter.GetOpacityTexture(core_, mtl_id));
		}

		public uint NumHistroyCmds()
		{
			return MtlEditorCoreImporter.NumHistroyCmds(core_);
		}
		public string HistroyCmdName(uint index)
		{
			return Marshal.PtrToStringAnsi(MtlEditorCoreImporter.HistroyCmdName(core_, index));
		}
		public uint EndCmdIndex()
		{
			return MtlEditorCoreImporter.EndCmdIndex(core_);
		}

		public void AmbientMaterial(uint mtl_id, Color value)
		{
			IntPtr ptr = this.ColorToIntPtr(value);
			MtlEditorCoreImporter.SetAmbientMaterial(core_, mtl_id, ptr);
			Marshal.FreeHGlobal(ptr);
		}
		public void DiffuseMaterial(uint mtl_id, Color value)
		{
			IntPtr ptr = this.ColorToIntPtr(value);
			MtlEditorCoreImporter.SetDiffuseMaterial(core_, mtl_id, ptr);
			Marshal.FreeHGlobal(ptr);
		}
		public void SpecularMaterial(uint mtl_id, Color value)
		{
			IntPtr ptr = this.ColorToIntPtr(value);
			MtlEditorCoreImporter.SetSpecularMaterial(core_, mtl_id, ptr);
			Marshal.FreeHGlobal(ptr);
		}
		public void ShininessMaterial(uint mtl_id, float value)
		{
			MtlEditorCoreImporter.SetShininessMaterial(core_, mtl_id, value);
		}
		public void EmitMaterial(uint mtl_id, Color value)
		{
			IntPtr ptr = this.ColorToIntPtr(value);
			MtlEditorCoreImporter.SetEmitMaterial(core_, mtl_id, ptr);
			Marshal.FreeHGlobal(ptr);
		}
		public void OpacityMaterial(uint mtl_id, float value)
		{
			MtlEditorCoreImporter.SetOpacityMaterial(core_, mtl_id, value);
		}
		public void DiffuseTexture(uint mtl_id, string name)
		{
			IntPtr ptr = Marshal.StringToHGlobalAnsi(name);
			MtlEditorCoreImporter.SetDiffuseTexture(core_, mtl_id, ptr);
			Marshal.FreeHGlobal(ptr);
		}
		public void SpecularTexture(uint mtl_id, string name)
		{
			IntPtr ptr = Marshal.StringToHGlobalAnsi(name);
			MtlEditorCoreImporter.SetSpecularTexture(core_, mtl_id, ptr);
			Marshal.FreeHGlobal(ptr);
		}
		public void ShininessTexture(uint mtl_id, string name)
		{
			IntPtr ptr = Marshal.StringToHGlobalAnsi(name);
			MtlEditorCoreImporter.SetShininessTexture(core_, mtl_id, ptr);
			Marshal.FreeHGlobal(ptr);
		}
		public void NormalTexture(uint mtl_id, string name)
		{
			IntPtr ptr = Marshal.StringToHGlobalAnsi(name);
			MtlEditorCoreImporter.SetNormalTexture(core_, mtl_id, ptr);
			Marshal.FreeHGlobal(ptr);
		}
		public void HeightTexture(uint mtl_id, string name)
		{
			IntPtr ptr = Marshal.StringToHGlobalAnsi(name);
			MtlEditorCoreImporter.SetHeightTexture(core_, mtl_id, ptr);
			Marshal.FreeHGlobal(ptr);
		}
		public void EmitTexture(uint mtl_id, string name)
		{
			IntPtr ptr = Marshal.StringToHGlobalAnsi(name);
			MtlEditorCoreImporter.SetEmitTexture(core_, mtl_id, ptr);
			Marshal.FreeHGlobal(ptr);
		}
		public void OpacityTexture(uint mtl_id, string name)
		{
			IntPtr ptr = Marshal.StringToHGlobalAnsi(name);
			MtlEditorCoreImporter.SetOpacityTexture(core_, mtl_id, ptr);
			Marshal.FreeHGlobal(ptr);
		}
		public uint SelectedMesh()
		{
			return MtlEditorCoreImporter.SelectedMesh(core_);
		}
		public void SelectMesh(uint mesh_id)
		{
			MtlEditorCoreImporter.SelectMesh(core_, mesh_id);
		}

		public void Undo()
		{
			MtlEditorCoreImporter.Undo(core_);
		}
		public void Redo()
		{
			MtlEditorCoreImporter.Redo(core_);
		}
		public void ClearHistroy()
		{
			MtlEditorCoreImporter.ClearHistroy(core_);
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
		private IntPtr ColorToIntPtr(Color clr)
		{
			float[] temp = new float[3];
			temp[0] = clr.R / 255.0f;
			temp[1] = clr.G / 255.0f;
			temp[2] = clr.B / 255.0f;
			for (int i = 0; i < 3; ++ i)
			{
				temp[i] = this.SRGBToLinear(temp[i]);
			}
			IntPtr ptr = Marshal.AllocHGlobal(sizeof(float) * 3);
			Marshal.Copy(temp, 0, ptr, 3);
			return ptr;
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
		private float SRGBToLinear(float srgb)
		{
			if (srgb < 0.04045f)
			{
				return srgb / 12.92f;
			}
			else
			{
				const float ALPHA = 0.055f;
				return (float)Math.Pow((srgb + ALPHA) / (1 + ALPHA), 2.4f);
			}
		}

		private IntPtr core_;
	}
}
