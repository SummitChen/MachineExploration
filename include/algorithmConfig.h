#pragma once
class algorithmConfig
{
public:
    algorithmConfig(){
        iSegment = (float)0.4;
        iGrid = (float)0.4;
        iFeature = (float)0.2;
        alpha = (float)0.4;
        beta = (float)0.6;
    }
    
    ~algorithmConfig(){}
    
    void setSegment(float value){
        if( value > 1.0 || value < 0.0){
            iSegment = 0.4;
        }else{
            iSegment = value;
        }
    }
    float getSegment(){
        return iSegment;
    }
    
    void setGrid(float value){
        if( value > 1.0 || value < 0.0){
            iGrid = 0.4;
        }else{
            iGrid = value;
        }
    }
    float getGrid(){
        return iGrid;
    }
    
    void setFeature(float value){
        if( value > 1.0 || value < 0.0){
            iFeature = 0.2;
        }else{
            iFeature = value;
        }
    }
    float getFeature(){
        return iFeature;
    }
    
    void setAlpha(float value){
        if( value > 1.0 || value < 0.0){
            alpha = 0.4;
        }else{
            alpha = value;
        }
    }
    float getAlpha(){
        return alpha;
    }
    
    void setBeta(float value){
        if( value > 1.0 || value < 0.0){
            beta = 0.6;
        }else{
            beta = value;
        }
    }
    float getBeta(){
        return beta;
    }
    
private:
    float iSegment;
    float iGrid;
    float iFeature;
    float alpha;
    float beta;
};

