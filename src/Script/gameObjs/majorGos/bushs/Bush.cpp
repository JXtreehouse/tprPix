/*
 * ======================= Bush.cpp ==========================
 *                          -- tpr --
 *                                        CREATE -- 2019.10.09
 *                                        MODIFY -- 
 * ----------------------------------------------------------
 */
#include "Script/gameObjs/majorGos/bushs/Bush.h"

//-------------------- CPP --------------------//
#include <functional>
#include <string>

//-------------------- Engine --------------------//
#include "Density.h"
#include "animSubspeciesId.h"
#include "dyParams.h"

#include "tprAssert.h"
#include "esrc_shader.h" 
#include "esrc_gameSeed.h"
#include "esrc_animFrameSet.h"


//-------------------- Script --------------------//
#include "Script/components/windAnim/WindAnim.h"

using namespace std::placeholders;

#include "tprDebug.h" 


namespace gameObjs {//------------- namespace gameObjs ----------------
namespace bush_inn {//----------- namespace: bush_inn ----------------//

    std::vector<AnimActionEName> windAnimActionEName_pool {};


    //----- flags -----//
    bool    isFstCalled {true};
    //===== funcs =====//
    void    init()noexcept;

}//-------------- namespace: bush_inn end ----------------//



class Bush_PvtBinary{
public:
    Bush_PvtBinary():
        windAnimUPtr( std::make_unique<component::WindAnim>() )
        {}

    Bush_PvtBinary( const Bush_PvtBinary & ){ tprAssert(0); }
    Bush_PvtBinary &operator=( const Bush_PvtBinary & );

    //------- vals -------//
    int tmp {0};
    std::unique_ptr<component::WindAnim> windAnimUPtr {};
};

Bush_PvtBinary &Bush_PvtBinary::operator=( const Bush_PvtBinary & ){
    tprAssert(0);
    return *this; // never reach
}





void Bush::init(GameObj &goRef_, const DyParam &dyParams_ ){

    //-- simple way ...
    if( bush_inn::isFstCalled ){
        bush_inn::isFstCalled = false;
        bush_inn::init();
    }

    //================ go.pvtBinary =================//
    auto *pvtBp = goRef_.init_pvtBinary<Bush_PvtBinary>();


    //================ dyParams =================//
    glm::dvec2          goMeshDPosOff {};


    size_t typeHash = dyParams_.get_typeHash();
    tprAssert( typeHash == typeid(DyParams_Blueprint).hash_code() );
    const DyParams_Blueprint *bpParamPtr = dyParams_.get_binaryPtr<DyParams_Blueprint>();
    const GoDataForCreate *goDataPtr = bpParamPtr->goDataPtr;
    tprAssert( !goDataPtr->isMultiGoMesh ); // must single gomesh
    const GoDataEntForCreate &goDataEntRef = *(*goDataPtr->goMeshDataUPtrs.cbegin()); // only one
    //pvtBp->subspeciesId = (*goDataPtr->goMeshDataUPtrs.cbegin())->subspeciesId;

    goMeshDPosOff = glm::dvec2{0.0, 0.0};


    //----- must before creat_new_goMesh() !!! -----//
    goRef_.actionDirection.reset( goDataPtr->direction );
    goRef_.brokenLvl.reset( goDataPtr->brokenLvl );


    //================ animFrameSet／animFrameIdxHandle/ goMesh =================//
        //-- 制作唯一的 mesh 实例: "root" --
        auto &goMeshRef = goRef_.creat_new_goMesh("root", //- gmesh-name
                                goDataEntRef.subspeciesId,
                                AnimActionEName::Idle,
                                RenderLayerType::MajorGoes, //- 不设置 固定zOff值
                                &esrc::get_shaderRef(ShaderType::UnifiedColor),  // pic shader
                                goMeshDPosOff, //- pposoff
                                0.0,  //- zOff
                                true //- isVisible
                                );

        // 先收集 所有参与 风吹动画的 gomesh 数据
        pvtBp->windAnimUPtr->insert_goMesh( &goMeshRef, goDataEntRef.windDelayIdx );

    
    //----- component: windAnim -----//
    // 正式 init
    pvtBp->windAnimUPtr->init( &bush_inn::windAnimActionEName_pool );

        
    //================ bind callback funcs =================//
    //-- 故意将 首参数this 绑定到 保留类实例 dog_a 身上
    goRef_.RenderUpdate = std::bind( &Bush::OnRenderUpdate,  _1 );   
    goRef_.LogicUpdate  = std::bind( &Bush::OnLogicUpdate,   _1 );

    //-------- actionSwitch ---------//
    goRef_.actionSwitch.bind_func( std::bind( &Bush::OnActionSwitch,  _1, _2 ) );
    goRef_.actionSwitch.signUp( ActionSwitchType::Idle );


    //================ go self vals =================//   

}


void Bush::OnRenderUpdate( GameObj &goRef_ ){
    //=====================================//
    //            ptr rebind
    //-------------------------------------//
    auto *pvtBp = goRef_.get_pvtBinaryPtr<Bush_PvtBinary>();


    //=====================================//
    //            windClock         
    //-------------------------------------//
    // goMeshRef.bind_animAction() 已经在 update 内被自动调用了
    // 这不是一个好实现 
    // 在未来，可能要被转移到 其他地方
    // ...
    pvtBp->windAnimUPtr->update();


    //=====================================//
    //  将 确认要渲染的 goMeshs，添加到 renderPool         
    //-------------------------------------//
    goRef_.render_all_goMesh();
}


void Bush::bind( GameObj &goRef_ ){ /* do nothing... */ }
void Bush::rebind( GameObj &goRef_ ){ /* do nothing... */ }
void Bush::OnLogicUpdate( GameObj &goRef_ ){ /* do nothing... */ }
void Bush::OnActionSwitch( GameObj &goRef_, ActionSwitchType type_ ){ /* do nothing... */ }





namespace bush_inn {//----------- namespace: bush_inn ----------------//



void init()noexcept{
    std::default_random_engine  randEngine;
    randEngine.seed( get_new_seed() );

    //=== windAnimActionName pool ===//
    windAnimActionEName_pool.reserve(20);
    windAnimActionEName_pool.insert( windAnimActionEName_pool.end(), 15, AnimActionEName::Wind );
    windAnimActionEName_pool.insert( windAnimActionEName_pool.end(), 1, AnimActionEName::Idle );
    std::shuffle( windAnimActionEName_pool.begin(), windAnimActionEName_pool.end(), randEngine );
}




}//-------------- namespace: bush_inn end ----------------//
}//------------- namespace gameObjs: end ----------------

