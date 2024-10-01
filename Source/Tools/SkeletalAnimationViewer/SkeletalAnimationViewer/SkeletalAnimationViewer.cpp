#include "PhotoshopAPI/include/PhotoshopAPI.h"
#include "Device.h"
#include "Window.h"
#include "ImGuiDevice.h"
#include "RenderManager.h"

#include "imgui.h"

void TestPhotoshopAPI()
{
	PhotoshopAPI::File inputFile = PhotoshopAPI::File("C:/Users/debgh/OneDrive/Documents/Unity Projects/Dreamwillow/Assets/Art/Characters/Player/PlayerHorizSprite.psb");

	PhotoshopAPI::PhotoshopFile* file = new PhotoshopAPI::PhotoshopFile();
	PhotoshopAPI::ProgressCallback callback;
	file->read(inputFile, callback);

	std::vector<PhotoshopAPI::LayerRecord>& layers = file->m_LayerMaskInfo.m_LayerInfo.m_LayerRecords;
	std::vector<PhotoshopAPI::ChannelImageData>& channels = file->m_LayerMaskInfo.m_LayerInfo.m_ChannelImageData;

	for (int i = 0; i < layers.size(); i++)
	{
		if (layers[i].getWidth() == 0 && layers[i].getHeight() == 0)
		{
			continue;
		}

		std::vector<uint8_t> data = channels[i].extractImageData<uint8_t>(0);
		std::cout << "Layer " << layers[i].m_LayerName.getString() << " has " << channels[i].getChannelOffsetsAndSizes().size()
			<< " channels. They have data with length " << data.size() << ", height "
			<< layers[i].getHeight() << ", and width " << layers[i].getWidth() << std::endl;
	}
}

int main()
{
	Window window;

	ImGuiDevice device;
	device.Setup(Window::GetWindowHandle(), Device::Get());

	RenderManager renderingManager;
	renderingManager.Setup();

	while (window.windowRunning)
	{
		window.Process();
		Device::Get()->StartFrame();
		device.StartFrame();
		renderingManager.StartFrame();

		ImGui::Begin("Test");
		ImGui::Text("Testing Windows");
		ImGui::End();

		renderingManager.EndFrame();
		device.EndFrame();
		Device::Get()->EndFrame();
	}
}