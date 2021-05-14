#pragma once
/***************************************************************************** /
/**
 * @file    dbApp.h
 * @author  Juan Miguel Cerda Ordaz
 * @date    13/05/2021
 * @brief   creates a graphical representation of the positions.
 */
/*****************************************************************************/

/*****************************************************************************/
/**
 * Includes
 */
/*****************************************************************************/

#include <SFML/Graphics.hpp>
#include "dbConnecter.h"
#include <map>


namespace dbTool {

  struct cmpVector3f {
    bool operator()(const sf::Vector3f& lhs, const sf::Vector3f& rhs) const {
      return lhs.x < rhs.x || 
             (lhs.x == rhs.x && (lhs.y < rhs.y || (lhs.y == rhs.y && lhs.z < rhs.z)));
    }
  };

  class dbApp
  {
   public:

    dbApp();

    ~dbApp();

    void 
    run();

   protected:

   private:

    void 
    start();

    void 
    update(float deltaTime);

    void
    render();

    void 
    shutdown();

    void
    initHeatMap();

    void 
    drawHeatMap();

    sf::Color
    BlendColor(sf::Color color1, sf::Color color2, float fraction);

    void
    calculateInfluence(int x, int y, float& maxInf, float& minInf);

    void
    propagateInfluence(sf::Vector2i pos);

    void
    setInfluence(sf::Vector2i pos, float influence);

    float
    getInfluence(float x, float y);

    //window
    sf::Vector2i m_resolution = {700,700};
    std::unique_ptr<sf::RenderWindow> m_window;

    //camera
    sf::View m_view;
    float m_cameraSpeed = 100.0f;

    //DB data
    dbConnecter m_connecter;
    std::vector<sf::Vector3f> m_positions;

    //input
    bool m_keyPressed = false;

    //heatmap
    std::map<sf::Vector3f, int, cmpVector3f> m_normalizedMap;
    vector<vector<float>> m_heatMap;
    float m_decay = 0.0035f;
    float m_momentum = 0.90f;

    //graphics
    sf::Texture m_heatRT;
    sf::Sprite m_heatSprite;
  };

}
