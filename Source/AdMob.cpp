/*
  ==============================================================================

    AdMob.cpp
    Created: 20 Sep 2018 11:29:23pm
    Author:  Charles Schiermeyer

  ==============================================================================
*/

#include "AdMob.h"
#define QUOTE_STRING(str) #str

#define STATIC_CALLBACK_IMPL(dataType, staticFunc, memberFunc ) \
void dataType::staticFunc(const firebase::Future<void> &result, void *userData) \
{ \
    if( result.status() == firebase::FutureStatus::kFutureStatusComplete && result.error() == 0 ) \
    {\
        if( dataType* callback = static_cast<dataType*>(userData) )\
        {\
            if( callback->memberFunc ) \
            { \
                callback->memberFunc(result); \
            } \
        } \
    } \
    else \
    { \
        DBG( QUOTE_STRING(dataType) << "::" << QUOTE_STRING(staticFunc) << " error: " << String(result.error_message() ) ); \
        if( dataType* callback =  static_cast<dataType*>(userData) ) \
        { \
            if( callback->errorCallback )\
            { \
                callback->errorCallback(result); \
            } \
        } \
    } \
}

#define STATIC_CALLBACK( a, b, c ) STATIC_CALLBACK_IMPL( a, b, c )
#define BANNER_STATIC_CALLBACK(b, c) STATIC_CALLBACK(BannerAdCallbacks, b, c)
#define INTERSTITIAL_STATIC_CALLBACK(b, c) STATIC_CALLBACK(InterstitialAdCallbacks, b, c)

#define MAKE_NAME(name) name##Callback

BANNER_STATIC_CALLBACK( MAKE_NAME(Init), MAKE_NAME(init) );
BANNER_STATIC_CALLBACK( MAKE_NAME(Load), MAKE_NAME(load) );
BANNER_STATIC_CALLBACK( MAKE_NAME(Hide), MAKE_NAME(hide) );
BANNER_STATIC_CALLBACK( MAKE_NAME(Show), MAKE_NAME(show) );
BANNER_STATIC_CALLBACK( MAKE_NAME(Pause), MAKE_NAME(pause) );
BANNER_STATIC_CALLBACK( MAKE_NAME(Resume), MAKE_NAME(resume) );
BANNER_STATIC_CALLBACK( MAKE_NAME(Destroy), MAKE_NAME(destroy) );
BANNER_STATIC_CALLBACK( MAKE_NAME(Move), MAKE_NAME(move) );

INTERSTITIAL_STATIC_CALLBACK( MAKE_NAME(Init), MAKE_NAME(init) );
INTERSTITIAL_STATIC_CALLBACK( MAKE_NAME(Load), MAKE_NAME(load) );
INTERSTITIAL_STATIC_CALLBACK( MAKE_NAME(Show), MAKE_NAME(show) );

#undef QUOTE_STRING
#undef STATIC_CALLBACK_IMPL
#undef STATIC_CALLBACK
#undef BANNER_STATIC_CALLBACK
#undef INTERSTITIAL_STATIC_CALLBACK
#undef MAKE_NAME

BannerAd::BannerAd(StringRef adID,
                   int adDuration,
                   int durationBetweenPopupsInSeconds,
                   bool useDefaultCallbacks) :
displayDurationInSeconds(adDuration),
pauseBetweenAdsInSeconds(durationBetweenPopupsInSeconds),
bannerAdUnitID(adID)
{
    if( useDefaultCallbacks)
    {
        setupDefaultCallbacks();
    }
}

BannerAd::~BannerAd()
{
    stopTimer(SpawnAd);
    stopTimer(DestroyAd);
}

void BannerAd::updateCallbacks(BannerAdCallbacks &newCallbacks)
{
    callbacks = newCallbacks;
}

void BannerAd::changeListenerCallback(juce::ChangeBroadcaster *source)
{
    startTimer(SpawnAd, 10);
}

void BannerAd::paint(Graphics& g)
{
    //g.fillAll(Colours::red);
}

void BannerAd::timerCallback(int timerID)
{
    switch( timerID )
    {
        case TimerIDs::SpawnAd:
        {
            stopTimer(TimerIDs::SpawnAd);
            
            firebase::admob::AdSize adSize;
            adSize.width = getWidth();
            adSize.height = getHeight();
            
            bannerView.reset( new firebase::admob::BannerView() );
            bannerView->Initialize(static_cast<firebase::admob::AdParent>(getWindowHandle()),
                                   bannerAdUnitID,
                                   adSize);
            bannerView->InitializeLastResult().OnCompletion(BannerAdCallbacks::InitCallback,
                                                            &callbacks);
            break;
        }
        case TimerIDs::DestroyAd:
        {
            stopTimer(DestroyAd);
            
            bannerView->Hide();
            bannerView->HideLastResult().OnCompletion(BannerAdCallbacks::HideCallback,
                                                      &callbacks);
            break;
        }
    }
}

void BannerAd::setupDefaultCallbacks()
{
    callbacks.initCallback = [this](const firebase::Future<void>& result)
    {
        bannerView->Show();
        bannerView->ShowLastResult().OnCompletion(BannerAdCallbacks::ShowCallback,
                                                  &callbacks);
        bannerView->SetListener(this);
    };
    
    callbacks.showCallback = [this](const firebase::Future<void>& result)
    {
        auto bounds = getScreenBounds();
        auto scale = Desktop::getInstance().getDisplays().getMainDisplay().scale;
        bannerView->MoveTo(bounds.getX() * scale, bounds.getY() * scale);
        bannerView->MoveToLastResult().OnCompletion(BannerAdCallbacks::MoveCallback,
                                                    &callbacks);
    };
    
    callbacks.moveCallback = [this](const firebase::Future<void>& result)
    {
        firebase::admob::AdRequest request = {};
        static const char* testIDs[] = {"kGADSimulatorID"};
        request.test_device_ids = testIDs;
        request.test_device_id_count = 1;
        bannerView->LoadAd(request);
        bannerView->LoadAdLastResult().OnCompletion(BannerAdCallbacks::LoadCallback,
                                                    &callbacks);
    };
    
    callbacks.loadCallback = [this](const firebase::Future<void>& result)
    {
        this->startTimer(TimerIDs::DestroyAd, displayDurationInSeconds * 1000);
    };
    
    callbacks.hideCallback = [this](const firebase::Future<void>& result)
    {
        bannerView->Destroy();
        bannerView->DestroyLastResult().OnCompletion(BannerAdCallbacks::DestroyCallback,
                                                     &callbacks);
    };
    
    callbacks.destroyCallback = [this](const firebase::Future<void>& result)
    {
        bannerView.reset(nullptr);
        this->startTimer(SpawnAd, pauseBetweenAdsInSeconds * 1000);
    };
}

void BannerAd::OnBoundingBoxChanged(firebase::admob::BannerView *banner_view, firebase::admob::BoundingBox box)
{
    if( boundingBoxChanged )
    {
        boundingBoxChanged(banner_view, box);
    }
}

void BannerAd::OnPresentationStateChanged(firebase::admob::BannerView *banner_view, firebase::admob::BannerView::PresentationState state)
{
    if( presentationStateChanged )
    {
        presentationStateChanged(banner_view, state);
    }
}
//==============================================================================
InterstitialAd::InterstitialAd(StringRef adID, int durationBetweenPopupsInSeconds, Component& owner) :
interstitialAdUnitID(adID),
duration(durationBetweenPopupsInSeconds)
{
    owner.addAndMakeVisible(this);
    startTimer(TimerIDs::Init, 1000);
}

InterstitialAd::~InterstitialAd()
{
    stopTimer(TimerIDs::Init);
    stopTimer(TimerIDs::SpawnAd);
}

void InterstitialAd::OnPresentationStateChanged(firebase::admob::InterstitialAd *interstitial_ad, firebase::admob::InterstitialAd::PresentationState state)
{
    if( state == firebase::admob::InterstitialAd::PresentationState::kPresentationStateHidden )
    {
        DBG( "ad has been hidden" );
        startTimer(TimerIDs::Init, 1000);
    }
    else if( state == firebase::admob::InterstitialAd::PresentationState::kPresentationStateCoveringUI)
    {
        DBG( "ad is covering UI" );
    }
}

void InterstitialAd::timerCallback(int timerID)
{
    switch (timerID)
    {
        case TimerIDs::Init:
        {
            stopTimer(TimerIDs::Init);
            
            interstitialView.reset( new firebase::admob::InterstitialAd() );
            
            callbacks.errorCallback = [this](const firebase::Future<void>& result)
            {
                DBG( "callbacks.errorCallback error! " << String(result.error_message()) );
                jassertfalse;
                this->stopTimer(TimerIDs::Init);
                this->stopTimer(TimerIDs::SpawnAd);
                this->startTimer(TimerIDs::Init, 10 * 1000);
            };
            
            callbacks.initCallback = [this](const firebase::Future<void>& result)
            {
                DBG( "callbacks.initCallback" );
                interstitialView->SetListener(this);
                firebase::admob::AdRequest request = {};
                static const char* testIDs[] = {"kGADSimulatorID"};
                request.test_device_ids = testIDs;
                request.test_device_id_count = 1;
                
                interstitialView->LoadAd(request);
                interstitialView->LoadAdLastResult().OnCompletion(InterstitialAdCallbacks::LoadCallback, &callbacks);
            };
            
            callbacks.loadCallback = [this](const firebase::Future<void>& result)
            {
                DBG( "callbacks.loadCallback" );
                this->startTimer(TimerIDs::SpawnAd, this->duration * 1000);
            };
            
            interstitialView->Initialize(static_cast<firebase::admob::AdParent>(getWindowHandle()),
                                         interstitialAdUnitID);
            interstitialView->InitializeLastResult().OnCompletion(InterstitialAdCallbacks::InitCallback, &callbacks);
            break;
        }
        case TimerIDs::SpawnAd:
        {
            stopTimer(TimerIDs::SpawnAd);
            if( auto* ad = interstitialView.get() )
            {
                //if ad initialized ok
                if( ad->InitializeLastResult().status() == firebase::FutureStatus::kFutureStatusComplete && ad->InitializeLastResult().error() == 0 )
                {
                    //if ad loaded ok
                    if( ad->LoadAdLastResult().status() == firebase::FutureStatus::kFutureStatusComplete && ad->LoadAdLastResult().error() == 0 )
                    {
                        //show it
                        ad->Show();
                    }
                    //else error message
                    else
                    {
                        DBG( "LoadAd error: " << String(ad->LoadAdLastResult().error_message()));
                        jassertfalse;
                    }
                }
                //else error message
                else
                {
                    DBG( "Initialize Error: " << String(ad->InitializeLastResult().error_message()));
                    jassertfalse;
                }
            }
            //else error message
            else
            {
                DBG( "no ad! " );
                jassertfalse;
            }
            break;
        }
    }
}










