#include <Geode/Geode.hpp>
#include <Geode/modify/CCEGLViewProtocol.hpp>
#include <Geode/modify/CCDirector.hpp>
#include <cmath>

class $modify(SkibidiDirector, cocos2d::CCDirector) {
public:
    static SkibidiDirector* sharedDirector() {
        return static_cast<SkibidiDirector*>(CCDirector::sharedDirector());
    }

    void createStatsLabel() {
        CCDirector::createStatsLabel();
    }
};

// completely reimpl the function from cocos2d source
class $modify(HookedCCEGLViewProtocol, cocos2d::CCEGLViewProtocol) {
    void setDesignResolutionSize(float width, float height, ResolutionPolicy resolutionPolicy) {
        auto resolutionPolicyUnderlying = static_cast<typename std::underlying_type<ResolutionPolicy>::type>(resolutionPolicy);
        geode::log::info("cocos2d::CCEGLViewProtocol::setDesignResolutionSize({}, {}, (ResolutionPolicy){})", width, height, resolutionPolicyUnderlying);

        CCAssert(resolutionPolicy != kResolutionUnKnown, "should set resolutionPolicy");
    
        if (width == 0.0f || height == 0.0f)
        {
            return;
        }

        m_obDesignResolutionSize.setSize(width, height);
        
        if (geode::Mod::get()->getSettingValue<bool>("enabled")) {
            m_fScaleX = geode::Mod::get()->getSettingValue<float>("scaleX");
            m_fScaleY = geode::Mod::get()->getSettingValue<float>("scaleY");
        } else {
            m_fScaleX = (float)m_obScreenSize.width / m_obDesignResolutionSize.width;
            m_fScaleY = (float)m_obScreenSize.height / m_obDesignResolutionSize.height;
        }

        if (!geode::Mod::get()->getSettingValue<bool>("disable-resolution-policy")) {
            if (resolutionPolicy == kResolutionNoBorder)
            {
                m_fScaleX = m_fScaleY = fmax(m_fScaleX, m_fScaleY);
            }
            
            if (resolutionPolicy == kResolutionShowAll)
            {
                m_fScaleX = m_fScaleY = fmin(m_fScaleX, m_fScaleY);
            }

            if ( resolutionPolicy == kResolutionFixedHeight) {
                m_fScaleX = m_fScaleY;
                m_obDesignResolutionSize.width = ceilf(m_obScreenSize.width/m_fScaleX);
            }

            if ( resolutionPolicy == kResolutionFixedWidth) {
                m_fScaleY = m_fScaleX;
                m_obDesignResolutionSize.height = ceilf(m_obScreenSize.height/m_fScaleY);
            }
        }
        
        // calculate the rect of viewport    
        float viewPortW = m_obDesignResolutionSize.width * m_fScaleX;
        float viewPortH = m_obDesignResolutionSize.height * m_fScaleY;

        m_obViewPortRect.setRect((m_obScreenSize.width - viewPortW) / 2, (m_obScreenSize.height - viewPortH) / 2, viewPortW, viewPortH);
        
        m_eResolutionPolicy = resolutionPolicy;
        
        // reset director's member variables to fit visible rect
        cocos2d::CCDirector::sharedDirector()->m_obWinSizeInPoints = getDesignResolutionSize();
        SkibidiDirector::sharedDirector()->createStatsLabel();
        cocos2d::CCDirector::sharedDirector()->setGLDefaultValues();
    }
};

#define LISTEN(name, type) geode::listenForSettingChanges(name, [](type) { \
    auto glView = cocos2d::CCDirector::sharedDirector()->getOpenGLView(); \
    glView->setDesignResolutionSize(480, 320, glView->m_eResolutionPolicy); \
});

$on_mod(Loaded) {
    LISTEN("enabled", bool);
    LISTEN("scaleX", float);
    LISTEN("scaleY", float);
    LISTEN("disable-resolution-policy", bool);
}
