import os
from django.shortcuts import render
from submit import models as submit_models
from polls import models as polls_models
from users import models as users_models
from comment import models as comment_models
from . import models
from django.db import connection
from bestcode import breadcrumbs

def activity(request, activity_id):
	submits = submit_models.Submit.objects.filter(activity_id=activity_id)
	plans = models.ActivityPlan.objects.filter(activity_id=activity_id)
	
	# review plan
	submit_review_plans = submit_models.SubmitReviewPlan.objects.filter(submit__activity_id=activity_id).order_by("plan_item_start")
	cur_activity = models.Activity.objects.get(activity_id=activity_id)
	comments = comment_models.Comment.objects.filter(comment_type__comment_type_name='activity').filter(object_id=activity_id)

	# add activity browse times
	cur_activity.browse_times += 1
	cur_activity.save()

	# set breadcrumbs nav
	breadcrumbs.JumpTo(request, cur_activity.name, request.get_full_path())
	request.session.modified = True

	# calc submit comment count
	for submit in submits:
		submit_comments = comment_models.Comment.objects.filter(comment_type__comment_type_name="submit").filter(object_id=submit.submit_id)
		submit.comment_count = len(submit_comments)

	# this is a ugly adaption to absolute path and relative path, i just dont want to explain why.
	for submit in submits:
		if submit.activity_user.photo.__str__()[0] != '/':
			submit.activity_user.photo = ("/media/%s" % submit.activity_user.photo)
		if submit.submit_reviewer.photo.__str__()[0] != '/':
			submit.submit_reviewer.photo = ("/media/%s" % submit.submit_reviewer.photo)

	context = {
		'nav_items': breadcrumbs.GetNavItems(request),
		'login': request.user.is_authenticated,
		'result': cur_activity.result,
		'submits': submits,
		'plans': plans,
		'submit_review_plans': submit_review_plans,
		'comments': comments, 
		'comment_path': "/comment/?comment_type=activity&object_id=%s&next='/activity/%s'" % (activity_id, activity_id),
	}
	return render(request, 'activities/activity.html', context)	
