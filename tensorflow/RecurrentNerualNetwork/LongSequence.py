import tensorflow as tf
import numpy as np

sentence = ("if you want to build a ship, don't drum up people together to "
           "collect wood and don't assign them tasks and work, but rather "
           "teach them to long for the endless immensity of the sea.")
char_set = list(set(sentence))
char_dic = {w: i for i, w in enumerate(char_set)}
data_dim = len(char_set)
hidden_size = len(char_set)
num_classes = len(char_set)
seq_len = 10

dataX = []
dataY = []

for i in range(0, len(sentence) - seq_len):
    x_str = sentence[i:i+seq_len]
    y_str = sentence[i+1:i+seq_len+1]
    x = [char_dic[c] for c in x_str]
    y = [char_dic[c] for c in y_str]
    dataX.append(x)
    dataY.append(y)

batch_size = len(dataX)

X = tf.placeholder(tf.int32, [None, seq_len])
Y = tf.placeholder(tf.int32, [None, seq_len])

X_one_hot = tf.one_hot(X, num_classes)

#RNN
cell = tf.nn.rnn_cell.BasicLSTMCell(num_units= hidden_size)
cell = tf.nn.rnn_cell.MultiRNNCell([cell]*2)

outputs, _states = tf.nn.dynamic_rnn(cell, X_one_hot, dtype= tf.float32)
X_for_softmax = tf.reshape(outputs, [-1, hidden_size])

# softmax layer
W = tf.get_variable("W", [hidden_size, num_classes])
b = tf.get_variable("b", [num_classes])
outputs = tf.matmul(X_for_softmax, W) + b

outputs = tf.reshape(outputs, [batch_size, seq_len, num_classes])

loss = tf.contrib.seq2seq.sequence_loss(logits= outputs, targets= Y, weights= tf.ones([batch_size, seq_len]))

train = tf.train.AdamOptimizer(learning_rate = 0.1).minimize(loss)

with tf.Session() as sess:
   sess.run(tf.global_variables_initializer())
   for i in range(500):
       l, _, results = sess.run([loss, train, outputs], feed_dict={X: dataX, Y: dataY})
       for j, result in enumerate(results):
           index = np.argmax(result, axis= 1)
           print(i, j, ''.join([char_set[t] for t in index]), l)
