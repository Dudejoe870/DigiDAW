#include "mainwindow.h"
#include <QApplication>
#include <QFile>
#include <QStyleFactory>

#include "main.h"

MainApplication::MainApplication()
{
	audioEngine = std::make_shared<DigiDAW::Audio::Engine>(RtAudio::Api::UNSPECIFIED);

	darkPalette.setColor(QPalette::Window, QColor(53, 53, 53));
	darkPalette.setColor(QPalette::WindowText, Qt::white);
	darkPalette.setColor(QPalette::Disabled, QPalette::WindowText, QColor(127, 127, 127));
	darkPalette.setColor(QPalette::Base, QColor(42, 42, 42));
	darkPalette.setColor(QPalette::AlternateBase, QColor(66, 66, 66));
	darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
	darkPalette.setColor(QPalette::ToolTipText, Qt::white);
	darkPalette.setColor(QPalette::Text, Qt::white);
	darkPalette.setColor(QPalette::Disabled, QPalette::Text, QColor(127, 127, 127));
	darkPalette.setColor(QPalette::Dark, QColor(35, 35, 35));
	darkPalette.setColor(QPalette::Shadow, QColor(20, 20, 20));
	darkPalette.setColor(QPalette::Button, QColor(53, 53, 53));
	darkPalette.setColor(QPalette::ButtonText, Qt::white);
	darkPalette.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(127, 127, 127));
	darkPalette.setColor(QPalette::BrightText, Qt::red);
	darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));
	darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
	darkPalette.setColor(QPalette::Disabled, QPalette::Highlight, QColor(80, 80, 80));
	darkPalette.setColor(QPalette::HighlightedText, Qt::white);
	darkPalette.setColor(QPalette::Disabled, QPalette::HighlightedText, QColor(127, 127, 127));

	DigiDAW::Audio::TrackState::BusIdentifier mainBus = audioEngine->trackState.addBus(
		DigiDAW::Audio::TrackState::Bus(
			DigiDAW::Audio::TrackState::ChannelNumber::Stereo,
			0.0f, 0.0f,
			std::vector<DigiDAW::Audio::TrackState::BusOutput> {  },
			std::vector<std::vector<unsigned int>>
	{
		std::vector<unsigned int> { 0 },
			std::vector<unsigned int> { 1 }
	})); // Make a Bus that outputs to the first two channels of the output device.

	audioEngine->trackState.addTrack(
		DigiDAW::Audio::TrackState::Track(
			DigiDAW::Audio::TrackState::ChannelNumber::Mono,
			-25.0f, 0.0f,
			std::vector<DigiDAW::Audio::TrackState::BusOutput>
	{
		DigiDAW::Audio::TrackState::BusOutput(mainBus, std::vector<std::vector<unsigned int>>
		{
			std::vector<unsigned int> { 0, 1 }
		})
	})); // Make a Mono Track that outputs both it's channels to the L and R channels of the mainBus.
}

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

	MainApplication mainApp;

    MainWindow win(&mainApp);
    win.show();

	app.setStyle(QStyleFactory::create("Fusion"));
	app.setPalette(mainApp.darkPalette);

    return app.exec();
}
