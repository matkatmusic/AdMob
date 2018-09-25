/*
  ==============================================================================

    This file was auto-generated!

  ==============================================================================
*/

#include "MainComponent.h"


//==============================================================================
MainContentComponent::MainContentComponent()
{
    auto area = Desktop::getInstance().getDisplays().getMainDisplay().userArea;
    setSize(area.getWidth(), area.getHeight());

    startTimer(1000);
}

void MainContentComponent::timerCallback()
{
    stopTimer();
    StringRef bannerAdUnit = "ca-app-pub-3940256099942544/2934735716"; //google test ad
    bannerAd.reset(new BannerAd(bannerAdUnit, 3, 4, true));
    addAndMakeVisible(bannerAd.get());
    addChangeListener(bannerAd.get());
    resized();
}




MainContentComponent::~MainContentComponent()
{

}

void MainContentComponent::paint (Graphics& g)
{
    g.fillAll (Colour (0xff001F36));

    g.setFont (Font (16.0f));
    g.setColour (Colours::white);
    g.drawText ("Hello World!", getLocalBounds(), Justification::centred, true);
}

void MainContentComponent::resized()
{
    // This is called when the MainContentComponent is resized.
    // If you add any child components, this is where you should
    // update their positions.
    if( bannerAd.get() )
    {
        bannerAd.get()->setBounds(getWidth() * 0.25f, getHeight() * 0.6f,
                                  getWidth() * 0.5f, getHeight() * 0.25f);
        sendChangeMessage();
    }
    
}
