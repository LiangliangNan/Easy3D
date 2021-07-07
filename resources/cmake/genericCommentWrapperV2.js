if(document.getElementById("IDCommentsPostTitle") && document.getElementById("IDCommentsPostTitle").innerHTML.length>0)
	idcomments_post_title = document.getElementById("IDCommentsPostTitle").innerHTML;
else
	idcomments_post_title = document.title;
if(idcomments_post_title.length==0)
	idcomments_post_title = "Post";
 	
idcomments_post_title = idcomments_post_title.replace(/#/, "%23");

if(null==idcomments_post_url || idcomments_post_url==='')
	idcomments_post_url = window.location.href;
if(null==idcomments_post_id)
	idcomments_post_id = window.location.href;
idcomments_post_id = encodeURIComponent(idcomments_post_id);
idcomments_post_url = encodeURIComponent(idcomments_post_url);
idcomments_post_title = encodeURIComponent(idcomments_post_title);
var commentScript = document.createElement("script");
commentScript.type = "text/javascript";
commentScript.src = "https://intensedebate.com/js/genericCommentWrapper2.php?acct="+idcomments_acct+"&postid="+idcomments_post_id+"&title="+idcomments_post_title+"&url="+idcomments_post_url;
document.getElementsByTagName("head")[0].appendChild(commentScript);
