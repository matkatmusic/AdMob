/*
  ==============================================================================

    AdMob.h
    Created: 20 Sep 2018 11:29:23pm
    Author:  Charles Schiermeyer

  ==============================================================================
*/

#ifndef ADMOB_H_INCLUDED
#define ADMOB_H_INCLUDED
#include "../JuceLibraryCode/JuceHeader.h"
#include "firebase/admob/interstitial_ad.h"
#include "firebase/admob/banner_view.h"

#define STATIC_CALLBACK_DECL(name) \
static void name##Callback(const firebase::Future<void>& result, void* userData);

#define FUNC_CALLBACK_DECL(name) \
std::function<void(const firebase::Future<void>&)> name##Callback

struct BannerAdCallbacks
{
    STATIC_CALLBACK_DECL(Init);
    STATIC_CALLBACK_DECL(Load);
    STATIC_CALLBACK_DECL(Hide);
    STATIC_CALLBACK_DECL(Show);
    STATIC_CALLBACK_DECL(Pause);
    STATIC_CALLBACK_DECL(Resume);
    STATIC_CALLBACK_DECL(Destroy);
    STATIC_CALLBACK_DECL(Move);
    
    FUNC_CALLBACK_DECL(init);
    FUNC_CALLBACK_DECL(load);
    FUNC_CALLBACK_DECL(hide);
    FUNC_CALLBACK_DECL(show);
    FUNC_CALLBACK_DECL(pause);
    FUNC_CALLBACK_DECL(resume);
    FUNC_CALLBACK_DECL(destroy);
    FUNC_CALLBACK_DECL(move);
    FUNC_CALLBACK_DECL(error);
};

struct BannerAd :
    public Component,
    public MultiTimer,
    public firebase::admob::BannerView::Listener,
    public ChangeListener
{
    BannerAd(StringRef adID, int adDuration, int durationBetweenPopupsInSeconds, bool useDefaultCallbacks);
    ~BannerAd();
    void timerCallback(int timerID) override;
    
    void paint(Graphics& g) override;
    
    void updateCallbacks(BannerAdCallbacks& newCallbacks);
    
    void changeListenerCallback(juce::ChangeBroadcaster *source) override;
    
    std::function<void(firebase::admob::BannerView *banner_view, firebase::admob::BoundingBox box)> boundingBoxChanged;
    std::function<void(firebase::admob::BannerView *banner_view, firebase::admob::BannerView::PresentationState state)> presentationStateChanged;
    
private:
    enum TimerIDs
    {
        SpawnAd,
        DestroyAd
    };
    
    int displayDurationInSeconds;
    int pauseBetweenAdsInSeconds;
    StringRef bannerAdUnitID;
    
    std::unique_ptr<firebase::admob::BannerView> bannerView;
    
    void OnBoundingBoxChanged(firebase::admob::BannerView *banner_view, firebase::admob::BoundingBox box) override;
    void OnPresentationStateChanged(firebase::admob::BannerView *banner_view, firebase::admob::BannerView::PresentationState state) override;
    
    void setupDefaultCallbacks();
    BannerAdCallbacks callbacks;
    
};
//==============================================================================
struct InterstitialAdCallbacks
{
    STATIC_CALLBACK_DECL(Init);
    STATIC_CALLBACK_DECL(Load);
    STATIC_CALLBACK_DECL(Show);

    FUNC_CALLBACK_DECL(init);
    FUNC_CALLBACK_DECL(load);
    FUNC_CALLBACK_DECL(show);
    FUNC_CALLBACK_DECL(error);
};

struct InterstitialAd :
    public Component,
    public MultiTimer,
    public firebase::admob::InterstitialAd::Listener
{
    InterstitialAd(StringRef adID, int durationBetweenPopupsInSeconds, Component& owner);
    ~InterstitialAd();
    
    void timerCallback(int timerID) override;
    void OnPresentationStateChanged(firebase::admob::InterstitialAd *interstitial_ad, firebase::admob::InterstitialAd::PresentationState state) override;
    InterstitialAdCallbacks callbacks;
private:
    StringRef interstitialAdUnitID;
    int duration;
    
    std::unique_ptr<firebase::admob::InterstitialAd> interstitialView;
    
    enum TimerIDs
    {
        Init,
        SpawnAd
    };
};
#undef STATIC_CALLBACK_DECL
#undef FUNC_CALLBACK_DECL
#endif  // ADMOB_H_INCLUDED
