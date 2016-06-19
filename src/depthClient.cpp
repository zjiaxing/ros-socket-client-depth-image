#include <ros/ros.h>
#include <image_transport/image_transport.h>
#include <cv_bridge/cv_bridge.h>
#include <sensor_msgs/image_encodings.h>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
using namespace std;;

class depthClient
{

  
public:
depthClient(): it_(nh_)
  {
	iniTcp();
    // Subscrive to input video feed 
    image_sub_depth = it_.subscribe("/camera/depth/image", 1, &depthClient::imageDepth, this);
   
  }

  ~depthClient()
  {
   closeTcp();
  }

  void imageDepth(const sensor_msgs::ImageConstPtr& msg);
  void iniTcp();
  void senddepth(cv::Mat dep);
  void closeTcp();
private:
  	ros::NodeHandle nh_;
  	image_transport::ImageTransport it_;
 	image_transport::Subscriber image_sub_depth;
  	image_transport::Publisher image_pub_;
  
  	cv::Mat depth,depthc;
  	int client_sockfd;
  	int depthlen;
  	struct sockaddr_in remote_addr; //server network address structure
  	char buf[BUFSIZ];  //buffer for data transfer

};
 
void depthClient::imageDepth(const sensor_msgs::ImageConstPtr& msg)
  {
    cv_bridge::CvImagePtr cvDepth_ptr;
    try
    {
      cvDepth_ptr = cv_bridge::toCvCopy(msg, "32FC1");
    }
    catch (cv_bridge::Exception& e)
    {
      ROS_ERROR("cvDepth_bridge exception: %s", e.what());
      return;
    }
	 depthc=cvDepth_ptr->image;
         depthc.convertTo(depth, CV_16UC1);
	 senddepth(depth);
       // cv::imshow("depthClient", depth);
       // cv::waitKey(3);
  }
void depthClient::iniTcp()
  {
	memset(&remote_addr,0,sizeof(remote_addr)); //Data initialization - clear
	remote_addr.sin_family=AF_INET; //Set to IP communication
	remote_addr.sin_addr.s_addr=inet_addr("127.0.0.1");//Server IP address
	remote_addr.sin_port=htons(8001); //Server port number
	
	/*Create client socket --IPv4 protocol, connection oriented communication, TCP protocol*/
	if((client_sockfd=socket(PF_INET,SOCK_STREAM,0))<0)
	{
		perror("socket");
		return ;
	}
	
	/*Bind the socket to the network address of the server*/
	if(connect(client_sockfd,(struct sockaddr *)&remote_addr,sizeof(struct sockaddr))<0)
	{
		perror("connect");
		return ;
	}
	printf("connected to server/n");
	
	
         return ;
	
  }

void depthClient::senddepth(cv::Mat dep)
{	
		printf("Enter depthimage to send:");		
		cv::Mat depk;
		dep.copyTo(depk); 
                cv::Mat depth=(depk.reshape(0,1));//to make it continuous
		int  depthSize = depth.total()*depth.elemSize();
		printf("send image:");
		depthlen=send(client_sockfd,depth.data,depthSize,0);
		
	//close(client_sockfd);//close socket
	return ;
  }
void depthClient::closeTcp()
{
	close(client_sockfd);//close socket
}
int main(int argc, char** argv)
{
  	ros::init(argc, argv, "depthClient");
 	
  	depthClient dc;

  	ros::spin();
	return 0;


         
}

