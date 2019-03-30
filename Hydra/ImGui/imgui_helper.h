#ifndef IMGUI_HELPER_H
#define IMGUI_HELPER_H

#include <string>
#include <vector>
#include "ImGui/imgui.h"
#include "Hydra/Core/ColorRGBA.h"

using namespace Hydra;

#define WSTAY (ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize)

static ImGuiID IMGUI_NOW_ID = 0;

typedef struct DraggingData {
	std::string id;
	void* data;
} DraggingData;

static DraggingData DragData;


static void BeginIds() {
	IMGUI_NOW_ID = 10000;
}

static ImGuiID GetNextId() {
	return IMGUI_NOW_ID++;
}

static const char* GetNextId(std::string label) {
	return (label + "##" + label + std::to_string(GetNextId())).c_str();
}

static bool OnButton(std::string name) {
	return ImGui::Button(GetNextId(name));
}

static const char* ComboData(std::vector<std::string> data) {
	std::string str = "";
	for (std::string s : data) {
		str += s + '\0';
	}
	return str.c_str();
}

static bool CanDropData(const std::string& id) {
	return ((ImGui::IsItemHovered() || ImGui::IsItemHoveredRect()) && DragData.id == id);
}

static bool IsDroppedData(const std::string& id, void*& data_out) {
	if (!ImGui::IsMouseDown(0) && (ImGui::IsItemHovered() || ImGui::IsItemHoveredRect()) && DragData.id == id) {
		data_out = DragData.data;
		return true;
	}
	return false;
}

static bool CreateDraggingObject(const std::string& id, void* data) {
	bool dragging = false;
	if (ImGui::IsMouseDown(0)) {
		if (ImGui::IsItemHovered() && DragData.id.length() == 0) {
			DragData.id = id;
			DragData.data = data;
		}
		if (DragData.id == id && DragData.data == data) {
			return true;
		}
	}
	return dragging;
}

static void RestartDragger() {
	if (!ImGui::IsMouseDown(0)) {
		DragData.id = "";
		DragData.data = nullptr;
	}
}

static bool DragInt(const std::string& label, int* v, int speed = 1.0f) {
	return ImGui::DragInt((label + "##" + std::to_string(GetNextId())).c_str(), v, speed);
}

static bool DragFloat(const std::string& label, float* v, float speed) {
	return ImGui::DragFloat((label + "##" + std::to_string(GetNextId())).c_str(), v, speed);
}

static bool DragFloat2(const std::string& label, float* v, float speed) {
	return ImGui::DragFloat2((label + "##" + std::to_string(GetNextId())).c_str(), v, speed);
}

static bool DragFloat3(const std::string& label, float* v, float speed) {
	return ImGui::DragFloat3((label + "##" + std::to_string(GetNextId())).c_str(), v, speed);
}

static bool DragFloat4(const std::string& label, float* v, float speed) {
	return ImGui::DragFloat4((label + "##" + std::to_string(GetNextId())).c_str(), v, speed);
}

static bool Checkbox(const std::string& label, bool* checked) {
	return ImGui::Checkbox((label + "##" + std::to_string(GetNextId())).c_str(), checked);
}

static bool CollapsingHeader(const std::string& label, bool default_open = true, int flags = 0) {
	return ImGui::CollapsingHeader((label + "##" + std::to_string(GetNextId())).c_str(), (default_open ? ImGuiTreeNodeFlags_DefaultOpen : 0) | flags);
}

static bool SelectableButton(const std::string& label, bool selected = false, ImGuiSelectableFlags flags = 0, const ImVec2& size = ImVec2(0, 0)) {
	return ImGui::Selectable((label + (std::string("##") + std::to_string(GetNextId()))).c_str(), selected, flags, size);
}

static void BeginFrame(int width, int height, bool border, int padding = -1) {
	if (padding >= 0) {
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { (float)padding, (float)padding });
	}
	ImGui::BeginChild(GetNextId(), { (float)width, (float)height }, border);
	if (padding >= 0) {
		ImGui::PopStyleVar();
	}
}

static void BeginFrame(int height, bool border, int padding = -1) {
	BeginFrame(0, height, border, padding);
}

static void BeginFrame(bool border, int padding = -1) {
	BeginFrame(0, border, padding);
}

static void EndFrame() {
	ImGui::EndChild();
}

static void BeginWindow(const std::string& name, int x, int y, int width, int height, bool stay = false, int padding = -1, bool* p_open = (bool*)0) {
	ImGui::SetNextWindowPos(ImVec2(x, y), stay ? 0 : ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(width, height), stay ? 0 : ImGuiCond_FirstUseEver);
	if (padding >= 0) {
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { (float)padding, (float)padding });
	}
	ImGui::Begin(name.c_str(), p_open, stay ? WSTAY : 0);
	if (padding >= 0) {
		ImGui::PopStyleVar();
	}
}

static void EndWindow() {
	ImGui::End();
}

static bool drawColorPicker(std::string label, float* v) {
	int misc_flags = (false ? ImGuiColorEditFlags_HDR : 0) | (false ? ImGuiColorEditFlags_AlphaPreviewHalf : (true ? ImGuiColorEditFlags_AlphaPreview : 0)) | (true ? 0 : ImGuiColorEditFlags_NoOptions);
	return ImGui::ColorEdit4(label.c_str(), v, misc_flags);
}

static bool drawColorPicker3(std::string label, float* v) {
	int misc_flags = (false ? ImGuiColorEditFlags_HDR : 0) | (false ? ImGuiColorEditFlags_AlphaPreviewHalf : (true ? ImGuiColorEditFlags_AlphaPreview : 0)) | (true ? 0 : ImGuiColorEditFlags_NoOptions);
	return ImGui::ColorEdit3(label.c_str(), v, misc_flags);
}

static void drawColorPicker(const char* name, ColorRGBA& c) {
	int misc_flags = (false ? ImGuiColorEditFlags_HDR : 0) | (false ? ImGuiColorEditFlags_AlphaPreviewHalf : (true ? ImGuiColorEditFlags_AlphaPreview : 0)) | (true ? 0 : ImGuiColorEditFlags_NoOptions);
	float color[4];
	color[0] = c.r;
	color[1] = c.g;
	color[2] = c.b;
	color[3] = c.a;
	if (ImGui::ColorEdit3(name, (float*)&color, misc_flags)) {
		c.r = color[0];
		c.g = color[1];
		c.b = color[2];
		c.a = color[3];
	}
}

static bool drawVectorInput(glm::vec3& vec, float prec = 0.1f) {
	bool e = false;
	e |= ImGui::DragFloat(GetNextId("X"), &vec.x, prec);
	e |= ImGui::DragFloat(GetNextId("Y"), &vec.y, prec);
	e |= ImGui::DragFloat(GetNextId("Z"), &vec.z, prec);
	return e;
}

static bool drawVectorInput2(glm::vec2& vec, float prec = 0.1f) {
	bool e = false;
	e |= ImGui::DragFloat(GetNextId("X"), &vec.x, prec);
	e |= ImGui::DragFloat(GetNextId("Y"), &vec.y, prec);
	return e;
}

static bool drawVectorInput4(glm::vec4& vec, float prec = 0.1f) {
	bool e = false;
	e |= ImGui::DragFloat(GetNextId("X"), &vec.x, prec);
	e |= ImGui::DragFloat(GetNextId("Y"), &vec.y, prec);
	e |= ImGui::DragFloat(GetNextId("Z"), &vec.z, prec);
	e |= ImGui::DragFloat(GetNextId("W"), &vec.w, prec);
	return e;
}

static bool drawVectorInput(std::string id, glm::vec3& vec, float prec = 0.1f) {
	bool e = false;
	e |= ImGui::DragFloat(("X##X" + id).c_str(), &vec.x, prec);
	e |= ImGui::DragFloat(("Y##Y" + id).c_str(), &vec.y, prec);
	e |= ImGui::DragFloat(("Z##Z" + id).c_str(), &vec.z, prec);
	return e;
}

static bool drawVectorInput2(std::string id, glm::vec2& vec, float prec = 0.1f) {
	bool e = false;
	e |= ImGui::DragFloat(("X##X" + id).c_str(), &vec.x, prec);
	e |= ImGui::DragFloat(("Y##Y" + id).c_str(), &vec.y, prec);
	return e;
}

static bool drawVectorInput4(std::string id, glm::vec4& vec, float prec = 0.1f) {
	bool e = false;
	e |= ImGui::DragFloat(("X##X" + id).c_str(), &vec.x, prec);
	e |= ImGui::DragFloat(("Y##Y" + id).c_str(), &vec.y, prec);
	e |= ImGui::DragFloat(("Z##Z" + id).c_str(), &vec.z, prec);
	e |= ImGui::DragFloat(("W##W" + id).c_str(), &vec.w, prec);
	return e;
}

static void DrawText(std::string str, const ColorRGBA& col = MakeRGB(255, 255, 25)) {
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(col.r, col.g, col.b, col.a));
	ImGui::Text(str.c_str());
	ImGui::PopStyleColor();
}

static void DrawText(const char* fmt, const ColorRGBA& col = MakeRGB(255, 255, 25)) {
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(col.r, col.g, col.b, col.a));
	ImGui::Text(fmt);
	ImGui::PopStyleColor();
}

static ImVec4 cRGBA(const ColorRGBA& col = MakeRGB(255, 255, 25)) {
	return ImVec4(col.r, col.g, col.b, col.a);
}

#endif // !IMGUI_HELPER_H