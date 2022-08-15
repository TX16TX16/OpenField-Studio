#include "Walnut/Application.h"
#include "Walnut/EntryPoint.h"

#include "Field.h"
#include "Filer.h"

#include <iostream>

#include "Walnut/Image.h"

class ExampleLayer : public Walnut::Layer
{
public:
	virtual void OnUIRender() override
	{
		
	
		ImGui::Begin("Count");
		ImGui::Text("Under Development");
		ImGui::End();
		
		ImGui::Begin("Player Properties");
		ImGui::Text("Under Development");
		ImGui::End();


		
		Field::ShowMainWindow();
		Field::ShowToolsWindow();
		Field::ShowInfoWindow();

		

	}
};


Walnut::Application* Walnut::CreateApplication(int argc, char** argv)
{
	Walnut::ApplicationSpecification spec;
	spec.Name = "Open Field";

	Filer::IntDef();
	

	Walnut::Application* app = new Walnut::Application(spec);
	app->PushLayer<ExampleLayer>();
	app->SetMenubarCallback([app]()
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Exit"))
			{
				app->Close();
			}
			ImGui::EndMenu();
		}
	});
	return app;
}